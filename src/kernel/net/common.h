#pragma once
#include "stdint.h"

typedef struct {
    uint8_t *data;
    uint8_t* payload;
    uint16_t length;
} NetPacket;

typedef uint8_t MACAddress[6];
typedef uint32_t IPv4Address;

#define MAC_BROADCAST (MACAddress) {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}
#define MAC_EMPTY (MACAddress) {0x0, 0x0, 0x0, 0x0, 0x0, 0x0}

#define ntohs(x) ((uint16_t)((x >> 8) | (x << 8)))
#define htons(x) ntohs(x)

#define MAX_ETH_SIZE 1518