//
// Created by an on 3/5/26.
//

#ifndef MQTT_UDP_TRANSPORT_BROKERSERVER_H
#define MQTT_UDP_TRANSPORT_BROKERSERVER_H
#include <cstring>
#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <span>
#include "../include/config.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <liburing.h>
#include <thread>

#include "Mem_manager.h"
#include "Workers.h"
#include "../include/Protocol.h"

using namespace std;
class BrokerServer {
    int epfd = -1;
    int notifyFd = -1;
    udp_packet_t *pool = nullptr;

    void spawn_worker(unsigned int core_id,  pid_t *workers_pid,  udp_packet_t *w_offset);

public:
    void start(unsigned int n);

    void stop();

    void restart();

protected:
    unsigned int getRequest();
};
#endif //MQTT_UDP_TRANSPORT_BROKERSERVER_H