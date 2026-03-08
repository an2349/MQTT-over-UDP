//
// Created by an on 2/15/26.
//

#ifndef BROKER_CONFIG_H
#define BROKER_CONFIG_H

#define MAX_TOPIC_LEN 128
#define MAX_CONTENT_LEN (10*1024*1024)
#define TIME_OUT 5000

#define MAX_WORKER 4

#define PORT 2003
#define MAX_CONN 100
#define MAX_EVENTS 1000

#define MAX_PACKAGE 4096
#define PACKAGE_SIZE 2048
//#define SLOT_SIZE sizeof(udp_slot_t)

#endif //BROKER_CONFIG_H