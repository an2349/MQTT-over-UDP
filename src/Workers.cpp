//
// Created by an on 3/3/26.
//
#include "Workers.h"

Workers::Workers(unsigned int core_id, udp_packet_t *w_offset, int ebpf_fd)
    : w_fd(-1), w_core_id(core_id), w_offset(w_offset), ebpf_fd(ebpf_fd) {
}

bool Workers::create(unsigned int core_id) {
    if ( !setup_any()) {
        std::cerr<<"F CPU"<<std::endl;
        return false;
    }
    if ( !setup_socket()) {
        std::cerr<<"F Socket ";
        return false;
    }
   if ( !init_uring()) {
       std::cerr<<"F Uring";
       return false;
   }
    std::cout<<"Succes at Worker "<<core_id<<std::endl;
    return true;
};
bool Workers::setup_any() {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(w_core_id % std::thread::hardware_concurrency(), &cpuset);
    if (sched_setaffinity(0, sizeof(cpuset), &cpuset)== -1) {
        std::cerr<<"F CPU "<<std::endl;
        return false;
    }
    return true;
}

bool Workers::setup_socket() {
    struct sockaddr_in address; //ipv6
    //int addrlen = sizeof(address);
    if ((w_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        //if ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << " loi o socket\n";
        return false;;
    }
    int opt = 1;
    if (setsockopt(w_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "setsockopt SO_REUSEADDR failed\n";
    }
    /*
    if (this->w_core_id == 0 && this->ebpf_fd > 0) {
        if (setsockopt(w_fd, SOL_SOCKET, SO_ATTACH_REUSEPORT_EBPF,
                       &this->ebpf_fd, sizeof(this->ebpf_fd)) < 0) {
            std::cerr << "Loi attach eBPF\n";
                       }
    }
    */
#ifdef SO_REUSEPORT
    if (setsockopt(w_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        std::cerr << "setsockopt SO_REUSEPORT failed\n";
    }
#endif

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(w_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
        std::cerr << "loi o bind port\n";
        return false;
    }

    /*if (listen(serverFd, MAX_CONN) < 0) {
        cerr << " loi lang nghe\n";
        return;
    }*/
    fcntl(w_fd, F_SETFL, fcntl(w_fd, F_GETFL, 0) | O_NONBLOCK);
    return true;
}

bool Workers::init_uring() {
    if (io_uring_queue_init(MAX_PACKAGE, &w_uring, IORING_SETUP_SQPOLL) < 0) {
        std::cerr<<"F IORING_SETUP_SQPOLL\n";
        return false;
    };

    struct iovec iov;
    iov.iov_base = w_offset;
    iov.iov_len  = MAX_PACKAGE * sizeof(udp_packet_t);

    if ( io_uring_register_buffers(&w_uring, &iov, 1) <0 ) {
        std::cerr<<"F IOURING_REGISTER_BUFFERS\n";
        return false;
    }

    struct io_uring_sqe *sqe;
    sqe = io_uring_get_sqe(&w_uring);
    if (sqe == NULL) {
        std::cerr<<"F IOURING_GET_SQE\n";
        return false;
    }
    io_uring_prep_provide_buffers(sqe, w_offset, sizeof(udp_packet_t), MAX_PACKAGE, w_core_id, 0);
    if ( io_uring_submit(&w_uring) < 0) {
        std::cerr<<"F IOURING_SUBMIT\n";
        return false;
    };
    return true;
}

void Workers::working(unsigned int id) {
    while (true) {
        sleep(1);
        std::cout<<id << " is working... \n";
    }
}