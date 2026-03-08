//
// Created by an on 3/3/26.
//

#ifndef MQTT_UDP_TRANSPORT_MEM_MANAGER_H
#define MQTT_UDP_TRANSPORT_MEM_MANAGER_H

#include <cstdio>
#include <cstring>
#include <sys/mman.h>
#include <fstream>
#include "../include/Protocol.h"

struct Mem_manager {
    uint8_t retry =0;
    static udp_packet_t *setup_mem_package_pool( Mem_manager& mm, size_t pool_size);
    static bool request_hugepages(int num_pages);
};
#endif //MQTT_UDP_TRANSPORT_MEM_MANAGER_H
