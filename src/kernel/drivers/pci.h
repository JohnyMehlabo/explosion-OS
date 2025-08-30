#pragma once
#include "stdbool.h"
#include "stdint.h"

bool PCI_FindDevice(uint16_t vendorId, uint16_t deviceId, uint8_t* bus, uint8_t* device, uint8_t* function);
bool PCI_FindDeviceByClass(uint8_t class, uint8_t subclass, uint8_t* bus, uint8_t* device, uint8_t* function);

uint16_t PCI_ConfigReadWord(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
void PCI_ConfigWriteWord(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint16_t data);

uint32_t PCI_ConfigReadLong(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
void PCI_ConfigWriteLong(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t data);


// Log information about connected PCI devices
void PCI_EnumerateDevices();