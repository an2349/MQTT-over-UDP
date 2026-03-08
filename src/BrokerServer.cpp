//
// Created by an on 2/14/26.
//

#include "BrokerServer.h"


void BrokerServer::start(unsigned int n) {
    Mem_manager mm;
    size_t pool_size = (size_t)n *  MAX_PACKAGE * sizeof(udp_packet_t);
    pool = mm.setup_mem_package_pool(mm, pool_size);
    if (!pool) {
        cerr << "Mem pool creation failed!" << endl;
        exit(1);
        return;
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
            if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                cerr << "Worker " << dead_pid << " crashed with code " << WEXITSTATUS(status) << endl;
            }
            for (unsigned int i = 0; i < n; ++i) {
                if (workers[i] == dead_pid) {
                    usleep(10000);
                    udp_packet_t* w_offset = pool + (i * MAX_PACKAGE);
                    spawn_worker(i, workers , w_offset);
                    break;
                }
            }
        }
    }
}

void BrokerServer::spawn_worker(unsigned int core_id, pid_t *workers_pid, udp_packet_t *w_offset) {
    pid_t pid = fork();
    if (pid == 0) {
        uint8_t exit_code = 0;
        Workers worker(core_id, w_offset);
        //exit(0);
        if ( !worker.create(core_id)) {
            cerr << "Create worker " << core_id << "fail"<<endl;
            exit_code = 1;
        }
        else {
            worker.working(core_id);
            exit_code = 0;
        }
        exit(exit_code);
    } else if (pid > 0) {
        workers_pid[core_id] = pid;
    } else {
        cerr<<"False in spawn worker "<<core_id;
    }
}