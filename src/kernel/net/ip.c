#include "ip.h"
#include "stdio.h"

typedef struct {
    uint8_t headerLen;
    uint8_t service;
    uint16_t totalLength;
    uint16_t identification;
    uint16_t fragmentOffsetAndFlags;
    uint8_t timeToLive;
    uint8_t protocol;
    uint16_t headerCheksum;
    IPv4Address sourceAddress;
    IPv4Address destAddress;
} __attribute__((packed)) IPv4Header;

// TODO: Won't do reassembly of fragments for now
// TODO: Header might contain more data in IP option. Dont care for now

void logIP(IPv4Address ip) {
    printf("%d.%d.%d.%d", ip & 0xff, (ip & 0xff00) >> 8, (ip & 0xff0000) >> 16, (ip & 0xff000000) >> 24);
}

void IP_HandlePacket(NetPacket packet, MACAddress srcMac) {
    // TODO: Here we should do some ARP caching

    IPv4Header* ipHeader = (IPv4Header*)packet.payload;
    uint8_t headerLength = (ipHeader->headerLen & 0xF) * 4;

    switch (ipHeader->protocol)
    {
    case 0x1:
        puts("ICMP packet from: ");
        logIP(ipHeader->sourceAddress);
        putc('\n');
        // ICMP_HandlePacket();
        break;
    default:
        break;
    }
}
