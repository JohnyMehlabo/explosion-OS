#pragma once
#include "disk.h"
#include "stddef.h"

typedef struct {
    uint8_t filename[11];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t creationTimeTenths;
    uint16_t creationTime;
    uint16_t creationDate;
    uint16_t lastAccessedDate;
    uint16_t startClusterHigh;
    uint16_t lastModificationTime;
    uint16_t lastModificationDate;
    uint16_t startClusterLow;
    uint32_t size;
} __attribute__((packed)) FAT_DirectoryEntry;

enum FAT_Attributes
{
    FAT_ATTRIBUTE_READ_ONLY         = 0x01,
    FAT_ATTRIBUTE_HIDDEN            = 0x02,
    FAT_ATTRIBUTE_SYSTEM            = 0x04,
    FAT_ATTRIBUTE_VOLUME_ID         = 0x08,
    FAT_ATTRIBUTE_DIRECTORY         = 0x10,
    FAT_ATTRIBUTE_ARCHIVE           = 0x20,
    FAT_ATTRIBUTE_LFN               = FAT_ATTRIBUTE_READ_ONLY | FAT_ATTRIBUTE_HIDDEN | FAT_ATTRIBUTE_SYSTEM | FAT_ATTRIBUTE_VOLUME_ID
};

typedef struct {
    uint32_t handle;
    bool isDirectory;
    uint32_t size;
    uint32_t position;
} FAT_File;

bool FAT_Initialize(DISK* disk);
FAT_File* FAT_Open(DISK* disk, const char* path);
void FAT_Close(FAT_File* file);
uint32_t FAT_Read(DISK* disk, FAT_File* file, size_t count, void* out);