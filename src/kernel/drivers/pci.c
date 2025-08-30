#include "pci.h"
#include "arch/i686/io.h"
#include "stdio.h"

#define PCI_CONFIG_ADDRESS 0xcf8
#define PCI_CONFIG_DATA 0xcfc

uint16_t PCI_ConfigReadWord(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t address = offset & 0xfc | 
                        (uint32_t)function << 8 | 
                        (uint32_t)device << 11 |
                        (uint32_t)bus << 16 |
                        0x80000000;

    i686_outl(PCI_CONFIG_ADDRESS, address);
    uint32_t input = i686_inl(PCI_CONFIG_DATA);
    return (uint16_t)((input >> ((offset & 2) * 8)) & 0xffff);
}

void PCI_ConfigWriteWord(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint16_t data) {
    uint32_t address = offset & 0xfc | 
                        (uint32_t)function << 8 | 
                        (uint32_t)device << 11 |
                        (uint32_t)bus << 16 |
                        0x80000000;

    i686_outl(PCI_CONFIG_ADDRESS, address);
    uint32_t input = i686_inl(PCI_CONFIG_DATA);
    uint32_t tmp = input & (0xFFFF0000 >> ((offset & 2) * 8)) | ((uint32_t)data) << ((offset & 2) * 8);
    i686_outl(PCI_CONFIG_DATA, tmp);
}

uint32_t PCI_ConfigReadLong(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t address = offset & 0xfc | 
                        (uint32_t)function << 8 | 
                        (uint32_t)device << 11 |
                        (uint32_t)bus << 16 |
                        0x80000000;

    i686_outl(PCI_CONFIG_ADDRESS, address);
    return i686_inl(PCI_CONFIG_DATA);
}

void PCI_ConfigWriteLong(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t data) {
    uint32_t address = offset & 0xfc | 
                        (uint32_t)function << 8 | 
                        (uint32_t)device << 11 |
                        (uint32_t)bus << 16 |
                        0x80000000;

    i686_outl(PCI_CONFIG_ADDRESS, address);
    i686_outl(PCI_CONFIG_DATA, data);
}

bool PCI_FindDevice(uint16_t vendorId, uint16_t deviceId, uint8_t* bus, uint8_t* device, uint8_t* function) {
    // Loop trough all buses, devices and functions to find a match
    for (uint16_t b = 0; b < 256; b++) {
        for (uint8_t d = 0; d < 32; d++) {
            for (uint8_t f = 0; f < 8; f++) {
                uint16_t currVendorId = PCI_ConfigReadWord(b, d, f, 0);
                uint16_t currDeviceId = PCI_ConfigReadWord(b, d, f, 2);
                // If we find a match return through the pointers the position of the device
                if (currVendorId == vendorId && currDeviceId == deviceId) {
                    *bus = b;
                    *device = d;
                    *function = f;
                    return true; // A match was found
                }
            }
        }
    }
    return false; // No matching devices
}

bool PCI_FindDeviceByClass(uint8_t class, uint8_t subclass, uint8_t* bus, uint8_t* device, uint8_t* function) {
    uint16_t targetClassInfo = ((uint16_t)class) << 8 | subclass;
    // Loop trough all buses, devices and functions to find a match
    for (uint16_t b = 0; b < 256; b++) {
        for (uint8_t d = 0; d < 32; d++) {
            for (uint8_t f = 0; f < 8; f++) {
                uint16_t currVendorId = PCI_ConfigReadWord(b, d, f, 0);
                uint16_t currDeviceId = PCI_ConfigReadWord(b, d, f, 2);
                uint16_t classInfo = PCI_ConfigReadWord(b, d, f, 0xa);

                // If we find a match return through the pointers the position of the device
                if (currVendorId != 0xffff && currDeviceId != 0xffff && classInfo == targetClassInfo) {
                    *bus = b;
                    *device = d;
                    *function = f;
                    return true; // A match was found 
                }
            }
        }
    }
    return false; // No matching devices
}

void PCI_EnumerateDevices() {
    // Loop trough all buses, devices and functions to find a match
    for (uint16_t b = 0; b < 256; b++) {
        for (uint8_t d = 0; d < 32; d++) {
            for (uint8_t f = 0; f < 8; f++) {
                uint16_t vendorId = PCI_ConfigReadWord(b, d, f, 0);
                uint16_t deviceId = PCI_ConfigReadWord(b, d, f, 2);
                if (vendorId != 0xffff) {
                    uint16_t classInfo = PCI_ConfigReadWord(b, d, f, 0xa);

                    printf("%x,%x,%x %x:%x Class: %d Subclass: %d\n", b, d, f, vendorId, deviceId, classInfo >> 8, classInfo & 0xff);
                }
            }
        }
    }
}

