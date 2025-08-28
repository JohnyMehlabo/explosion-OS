#include "disk.h"
#include "x86.h"
#include "stdio.h"

bool DISK_InitializeDisk(uint8_t driveNumber, DISK* diskOut) {
    uint8_t driveType;
    uint16_t cylinders, heads, sectors;
    
    if (!x86_Disk_GetDriveParams(driveNumber, &driveType, &cylinders, &sectors, &heads))
        return false;

    diskOut->id = driveNumber;
    diskOut->cylinders = cylinders;
    diskOut->heads = heads;
    diskOut->sectors = sectors;

    return true;
}

void DISK_LBA2CHS(DISK* disk, uint32_t lba, uint16_t* cylinderOut, uint16_t* headOut, uint16_t* sectorOut) {
    *cylinderOut = lba / (disk->heads * disk->sectors);
    *headOut = (lba / disk->sectors) % disk->heads;
    *sectorOut = (lba % disk->sectors) + 1;
}

bool DISK_ReadSectors(DISK* disk, uint32_t lba, uint8_t count, void* bufferOut) {

    uint16_t cylinder, head, sector;

    DISK_LBA2CHS(disk, lba, &cylinder, &head, &sector);
    
    for (int i = 0; i < 3; i++) {
        if (x86_Disk_Read(disk->id, cylinder, sector, head, count, bufferOut))
            return true;
    }
    
    return false;
}