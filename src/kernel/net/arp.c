#include "arp.h"
#include "stdio.h"
#include "ethernet.h"
#include "interfaces.h"
#include "memory.h"

typedef struct {
    uint16_t hardwareType;
    uint16_t protocolType;
    uint8_t hardwareLength;
    uint8_t protocolLength;
    uint16_t operation;
    MACAddress senderHardwareAddress;
    IPv4Address senderProtocolAddress;
    MACAddress targetHardwareAddress;
    IPv4Address targetProtocolAddress;
} __attribute__((packed)) ARPPacket;


void logMAC(const MACAddress* address) {
    printf("%x:%x:%x:%x:%x:%x", (*address)[0], (*address)[1], (*address)[2], (*address)[3], (*address)[4], (*address)[5]);
}

#define HARDWARE_TYPE_ETHERNET 1
#define PROTOCOL_TYPE_IPv4 0x800
#define OPERATION_REQUEST 1
#define OPERATION_REPLY 2

void ARP_SendReply(ARPPacket* arpPacket, IPv4Address localIP) {
    NetPacket packet = Ethernet_CreatePacket(arpPacket->senderHardwareAddress, 0x806, sizeof(ARPPacket));
    ARPPacket* payload = (ARPPacket*)packet.payload;
    
    payload->hardwareType = htons(HARDWARE_TYPE_ETHERNET);
    payload->protocolType = htons(PROTOCOL_TYPE_IPv4);
    payload->hardwareLength = 6;
    payload->protocolLength = 4;
    payload->operation = htons(OPERATION_REPLY);
    Ethernet_GetMACAddress(payload->senderHardwareAddress);
    payload->senderProtocolAddress = localIP;
    memcpy(payload->targetHardwareAddress, arpPacket->senderHardwareAddress, sizeof(MACAddress));
    payload->targetProtocolAddress = arpPacket->senderProtocolAddress;
    puts("Resolving my IP\n");
    Ethernet_SendPacket(packet);
}

void ARP_SendRequest(IPv4Address destAddress) {
    NetPacket packet = Ethernet_CreatePacket(MAC_BROADCAST, 0x806, sizeof(ARPPacket));

    ARPPacket* payload = (ARPPacket*)packet.payload;
    payload->hardwareType = htons(HARDWARE_TYPE_ETHERNET);
    payload->protocolType = htons(PROTOCOL_TYPE_IPv4);
    payload->hardwareLength = 6;
    payload->protocolLength = 4;
    payload->operation = htons(OPERATION_REQUEST);
    Ethernet_GetMACAddress(payload->senderHardwareAddress);
    payload->senderProtocolAddress = Interfaces_GetLocalIP();
    memcpy(payload->targetHardwareAddress, MAC_EMPTY, sizeof(MACAddress));
    payload->targetProtocolAddress = destAddress;
    Ethernet_SendPacket(packet);
}

void ARP_HandlePacket(NetPacket packet) {
    ARPPacket* arpPacket = (ARPPacket*)packet.data;
    puts("ARP: ");
    logMAC(&arpPacket->senderHardwareAddress);
    putc(' ');
    logMAC(&arpPacket->targetHardwareAddress);
    putc('\n');

    uint32_t operation = ntohs(arpPacket->operation);
    if (operation == OPERATION_REQUEST) {

        IPv4Address localIP = Interfaces_GetLocalIP();

        if (arpPacket->targetProtocolAddress == localIP) {
            ARP_SendReply(arpPacket, localIP);
        }

    } else if (operation == OPERATION_REPLY) {

    }

}