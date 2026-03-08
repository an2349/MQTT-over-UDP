//
// Created by an on 3/3/26.
//

#ifndef BROKER_PROTOCOL_H
#define BROKER_PROTOCOL_H
#include <cstdint>

#include "config.h"

struct connection_stream {
};

struct connection_state {
    uint8_t type;
    uint32_t conn_id; // 4 bytes
    uint32_t stream_id; //4
    uint16_t topic_len; // 2 bytes
    uint16_t last_active; // 2 bytes
    uint32_t package_number; // 4 bytes
    uint8_t request_type; // 1 byte
    uint16_t index; //2
} __attribute__((aligned(32)));


struct mqtt_packet {
    uint8_t type;
    uint32_t length;
    uint16_t topicLen;
    uint32_t topicName;
    uint8_t payload[PACKAGE_SIZE];
};

struct __attribute__((packed)) mqtt_header {
    uint8_t qos;
    uint8_t retained;
    uint16_t packetId;
    uint32_t packetLength;
};

struct __attribute__((packed)) udp_header {
    uint16_t src_port;
    uint16_t dst_port;
    uint16_t length;
    uint16_t checksum;
};
struct __attribute__((packed))  my_package_header{
    uint8_t flags; //1
    uint32_t conn_id ;//4
    uint32_t seq; //4
    uint8_t auth_tag[16]; //16

};
/*typedef struct {
    udp_header udp_header;//8
    my_package_header package_header;
    uint8_t padding[52];
    uint8_t payload[1984]; //Payload
} __attribute__((aligned(64))) udp_packet_t;*/

typedef struct {
    uint8_t raw[PACKAGE_SIZE];
} __attribute__((aligned(64))) udp_packet_t;

#endif //BROKER_PROTOCOL_H