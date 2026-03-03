//
// Created by an on 2/14/26.
//
#include <cstring>
#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <span>
#include "include/config.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <liburing.h>
#include <thread>

using namespace std;


class BrokerServer {
private:
    int epfd = -1;
    int notifyFd = -1;

    struct connection_stream {
    };

    struct connection_state {
        // bit 0: is_auth
        // bit 1: is_encrypted
        // bit 2-7: reserved
        uint8_t flags; // 1 byte for Auth
        uint32_t conn_id; // 4 bytes
        uint32_t stream_id; //4
        uint16_t topic_len; // 2 bytes
        uint16_t last_active; // 2 bytes (Timestamp)
        uint32_t package_number; // 4 bytes
        uint8_t request_type; // 1 byte
        uint16_t index; //2
    } __attribute__((aligned(32)));

    struct raw_mqtt_packet {
        uint8_t type;
        uint16_t length;
        uint16_t topicLen;
        string_view topicName;
        span<uint8_t> payload;
    };

    struct mqtt_packet {
        uint8_t type;
        uint32_t length;
        uint16_t topicLen;
        string_view topicName;
        span<uint8_t> payload;
    };

    typedef struct {
        uint32_t conn_id; // 4
        uint32_t src_ip; // 4
        uint16_t src_port; // 2
        uint16_t packet_len; // 2
        uint64_t seq_num; // 8
        uint8_t status; // 1 (0: EMPTY, 1: READY, 2: WORKING)
        uint8_t padding[43]; // Padding
        uint8_t payload[1984]; // more
    } __attribute__((aligned(64))) udp_packet_t;



    udp_packet_t *setup_mem_package_pool(size_t pool_size) {
        void *ptr = mmap(NULL, pool_size,
                         PROT_READ | PROT_WRITE,
                         MAP_ANONYMOUS | MAP_POPULATE | MAP_SHARED | MAP_HUGETLB,
                         -1, 0);

        if (ptr == MAP_FAILED) {
            perror("Cannot hugepage,use default page");
            ptr = mmap(NULL, pool_size,
                       PROT_READ | PROT_WRITE,
                       MAP_ANONYMOUS | MAP_POPULATE | MAP_SHARED,
                       -1, 0);

            if (ptr == MAP_FAILED) {
                perror("Cannot mmap");
                return nullptr;
            }
        }
        madvise(ptr, pool_size, MADV_WILLNEED | MADV_SEQUENTIAL);
        memset(ptr, 0, pool_size);

        return static_cast<udp_packet_t *>(ptr);
    }

    udp_packet_t *pool = nullptr;


    void even_loop(unsigned int n ,udp_packet_t *w_offset);

    void spawn_worker(unsigned int n, pid_t *worker_pid ,udp_packet_t *w_offset);

public:
    void start(unsigned int n);

    void stop();

    void restart();

protected:
    unsigned int getRequest();
};

void BrokerServer::start(unsigned int n) {
    //tao be de du lieu udp do vao
    size_t pool_size = (size_t)n *  MAX_PACKAGE * sizeof(udp_packet_t);
    pool = setup_mem_package_pool(pool_size);
    if (!pool) {
        exit(1);
    }

    pid_t workers[n];
    for (unsigned int core_id = 0; core_id < n; ++core_id) {
        udp_packet_t* w_offset = pool + (core_id * MAX_PACKAGE);
        spawn_worker(core_id, workers, w_offset);
    }
    while (true) {
        int status;
        pid_t dead_pid = wait(&status);
        if (dead_pid > 0) {
            for (unsigned int i = 0; i < n; ++i) {
                if (workers[i] == dead_pid) {
                    udp_packet_t* w_offset = pool + (i * MAX_PACKAGE);
                    spawn_worker(i, workers , w_offset);
                    break;
                }
            }
        }
    }
}

void BrokerServer::even_loop(unsigned int core_id, udp_packet_t *w_offset) {
    cpu_set_t cpuset;
    int worker_fd = -1;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id % std::thread::hardware_concurrency(), &cpuset);
    sched_setaffinity(0, sizeof(cpuset), &cpuset);
    struct sockaddr_in address; //ipv4
    int addrlen = sizeof(address);

    if ((worker_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        //if ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cerr << " loi o socket\n";
        return;
    }
    int opt = 1;
    if (setsockopt(worker_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        cerr << "setsockopt SO_REUSEADDR failed\n";
    }
#ifdef SO_REUSEPORT
    if (setsockopt(worker_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        cerr << "setsockopt SO_REUSEPORT failed\n";
    }
#endif

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(worker_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
        cerr << "loi o bind port\n";
        return;
    }

    /*if (listen(serverFd, MAX_CONN) < 0) {
        cerr << " loi lang nghe\n";
        return;
    }*/
    fcntl(worker_fd, F_SETFL, fcntl(worker_fd, F_GETFL, 0) | O_NONBLOCK);

    struct io_uring ring;
    io_uring_queue_init(MAX_PACKAGE, &ring, IORING_SETUP_SQPOLL);

    struct iovec iov;
    iov.iov_base = w_offset;
    iov.iov_len  = MAX_PACKAGE * sizeof(udp_packet_t);

    io_uring_register_buffers(&ring, &iov, 1);

    struct io_uring_sqe *sqe;
    sqe = io_uring_get_sqe(&ring);
    io_uring_prep_provide_buffers(sqe, w_offset, sizeof(udp_packet_t), MAX_PACKAGE, core_id, 0);
    io_uring_submit(&ring);
}

void BrokerServer::spawn_worker(unsigned int core_id, pid_t *workers_pid, udp_packet_t *w_offset) {
    pid_t pid = fork();
    if (pid == 0) {
        even_loop(core_id, w_offset);
        exit(0);
    } else if (pid > 0) {
        workers_pid[core_id] = pid;
    } else {
        cerr<<"False in spawn worker "<<core_id;
    }
}
