#pragma once
#include "stdint.h"
#include "stdbool.h"

typedef struct {
    uint8_t id;
    uint16_t cylinders;
    uint16_t heads;
    uint16_t sectors;
} DISK;

bool DISK_InitializeDisk(uint8_t driveNumber, DISK* diskOut);
void DISK_LBA2CHS(DISK* disk, uint32_t lba, uint16_t* cylinderOut, uint16_t* headOut, uint16_t* sectorOut);
bool DISK_ReadSectors(DISK* disk, uint32_t lba, uint8_t count, void* bufferOut);