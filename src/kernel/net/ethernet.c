#include "ethernet.h"
#include "memory.h"
#include "stdio.h"
#include "drivers/net/rtl8139.h"
#include "arp.h"
#include "ip.h"

MACAddress macAddr;

typedef struct {
    MACAddress destAddr;
    MACAddress srcAddr;
    uint16_t etherType;
} __attribute__((packed)) EthernetFrameHeader;


void Ethernet_SetMACAddress(MACAddress addr) {
    memcpy(macAddr, addr, sizeof(MACAddress));
}

void Ethernet_GetMACAddress(MACAddress out) {
    memcpy(out, macAddr, sizeof(MACAddress));
}

void Ethernet_HandlePacket(NetPacket packet) {
    EthernetFrameHeader* header = (EthernetFrameHeader*)packet.data;

    uint16_t etherType = ntohs(header->etherType);
    
    NetPacket strippedPacket;
    strippedPacket.data = packet.data  + sizeof(EthernetFrameHeader);
    strippedPacket.payload = strippedPacket.data;
    strippedPacket.length = packet.length - sizeof(EthernetFrameHeader);

    switch (etherType)
    {
    case 0x0806:
        ARP_HandlePacket(strippedPacket);
        break;
    case 0x0800:
        IP_HandlePacket(strippedPacket, header->srcAddr);
        break;
    default:
        break;
    }
}

uint8_t packetBuffer[MAX_ETH_SIZE];
NetPacket Ethernet_CreatePacket(MACAddress dest, uint16_t etherType, uint32_t payloadSize) {
    NetPacket createdPacket;
    createdPacket.data = packetBuffer;
    createdPacket.payload = packetBuffer + sizeof(EthernetFrameHeader);
    createdPacket.length = sizeof(EthernetFrameHeader) + payloadSize;

    EthernetFrameHeader* header = (EthernetFrameHeader*)createdPacket.data;
    memcpy(header->destAddr, dest, sizeof(MACAddress));
    header->etherType = htons(etherType);
    memcpy(header->srcAddr, macAddr, sizeof(MACAddress));

    return createdPacket;
}

void Ethernet_SendPacket(NetPacket packet) {
    RTL8139_SendPacket(packet);
}
