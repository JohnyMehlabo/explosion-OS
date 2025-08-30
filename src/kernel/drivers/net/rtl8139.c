#include "rtl8139.h"
#include "arch/i686/io.h"
#include "stdint.h"
#include "../pci.h"
#include "stdio.h"
#include "memory.h"
#include "arch/i686/irq.h"
#include "net/ethernet.h"
#include "net/common.h"

#define VENDOR_ID 0x10ec 
#define DEVICE_ID 0x8139 // NOTE: This device ID only represents qemuy, kvm and virtual box emulated NICs

#define ISR_TOK 0x4
#define ISR_ROK 0x1

uint16_t ioaddr;
MACAddress macAddress;

uint32_t curOffset = 0;
__attribute__((aligned(4096))) uint8_t rxBuffer[8192 + 16];

void RTL8139_IRQHandler() {
    uint16_t status = i686_inw(ioaddr + 0x3e);
    i686_outw(ioaddr + 0x3e, 0x05);

    if(status & ISR_TOK) {
		// Sent
        puts("Sent packet\n");
	}
	if (status & ISR_ROK) {
		// Received packet
        uint8_t* currentPtr = (uint8_t*)rxBuffer + curOffset;

        uint16_t pkgStatus = *(uint16_t*)currentPtr;
        uint16_t length = *(uint16_t*)(currentPtr + 2);

        // puts("Received packet with size: ");
        // putd(length);
        
        uint8_t packet[MAX_ETH_SIZE];
        
        uint32_t pkgStart = curOffset + 4;
        uint8_t* pkgStartPtr = rxBuffer + pkgStart;

        if (pkgStart + length > sizeof(rxBuffer)) { // Wraps around end
            uint32_t lengthBeforeWrap = curOffset + length - sizeof(rxBuffer);
            memcpy(packet, pkgStartPtr, lengthBeforeWrap ); // Copy part before wrap
            memcpy(packet + lengthBeforeWrap, rxBuffer, length-lengthBeforeWrap); // Copy part that wrapped from the beginning
        } else {
            memcpy(packet, pkgStartPtr, length);
        }

        curOffset = (curOffset + length + 4 + 3) & ~3;
        curOffset %= sizeof(rxBuffer);
        i686_outw(ioaddr + 0x38, curOffset - 16);

        Ethernet_HandlePacket((NetPacket){ .data=packet, .payload=packet, .length=length });
	}
}

void RTL8139_Init() {
    uint8_t bus;
    uint8_t device;
    uint8_t function;
    
    if (!PCI_FindDevice(VENDOR_ID, DEVICE_ID, &bus, &device, &function)) {
        puts("Error while initializing RTL8139. Device not found\n");
        return;
    } 
    
    // Get base IO address from PCI
    ioaddr = PCI_ConfigReadLong(bus, device, function, 0x10) & 0xFFFC;

    // Enable bus mastering
    uint32_t command = PCI_ConfigReadLong(bus, device, function, 0x4);
    command |= 0b100;
    PCI_ConfigWriteLong(bus, device, function, 0x4, command);

    // Retrieve MAC address
    for (int i = 0; i < 6; i++) {
        uint8_t macValue = i686_inb(ioaddr+i);
        putd(macValue);
        putc(':');
        macAddress[i] = macValue;
    }

    Ethernet_SetMACAddress(macAddress);

    // Power on
    i686_outb( ioaddr + 0x52, 0x0);

    // Software reset
    i686_outb( ioaddr + 0x37, 0x10);
    while ((i686_inb(ioaddr + 0x37) & 0x10) != 0) { }

    // Set RX Buffer
    i686_outl(ioaddr + 0x30, (uintptr_t)rxBuffer  - 0xc0000000);

    // Enable interrupts for TOK and ROK
    i686_outw(ioaddr + 0x3C, 0x5); 

    // WRAP = 0 (Wrap around if overflow)
    // Accept: Broadcast, Physical Match
    i686_outw(ioaddr + 0x44, 0xa);

    // Enable receiving and transmitting (RE and TE)
    i686_outb(ioaddr + 0x37, 0x0c);

    uint8_t interruptLine = PCI_ConfigReadWord(bus, device, function, 0x3c) & 0xff;

    i686_IRQ_SetHandler(interruptLine, RTL8139_IRQHandler);    
}

#define TRANSMIT_REG_COUNT 4
#define TRANSMIT_REG_SIZE 4
#define TRANSMIT_STATUS_REG_OFFSET 0x10
#define TRANSMIT_START_REG_OFFSET 0x20

uint8_t transmitRegister = 0;
uint8_t transmitBuffers[4][MAX_ETH_SIZE];

bool RTL8139_SendPacket(NetPacket packet) {
    uint32_t currentStatus = i686_inl(ioaddr + TRANSMIT_STATUS_REG_OFFSET + transmitRegister);

    if (!(currentStatus & 1 << 13)) {
        puts("Error while sending packet. Space not available\n");
        return false;
    }

    memcpy(transmitBuffers[transmitRegister], packet.data, MAX_ETH_SIZE);

    i686_outl(ioaddr + TRANSMIT_START_REG_OFFSET + transmitRegister, (uint32_t)(transmitBuffers[transmitRegister] - 0xc0000000));
    i686_outl(ioaddr + TRANSMIT_STATUS_REG_OFFSET + transmitRegister, packet.length & 0x1fff);

    transmitRegister += TRANSMIT_REG_SIZE;
    transmitRegister %= TRANSMIT_REG_COUNT * TRANSMIT_REG_SIZE;
}