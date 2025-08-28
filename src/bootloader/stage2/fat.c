#include "fat.h"
#include "disk.h"
#include "memdefs.h"
#include "stdio.h"
#include "stddef.h"
#include "memory.h"
#include "string.h"
#include "ctype.h"
#include "minmax.h"

#define ROOT_DIRECTORY_HANDLE -1
#define SECTOR_SIZE 512
#define MAX_FILE_HANDLES 10
#define MAX_PATH_SIZE 260

typedef struct {
    uint8_t JumpInst [3];
    uint8_t OEMIdentifier[8];
    uint16_t BytesPerSector;
    uint8_t SectorsPerCluster;
    uint16_t ReservedSectors;
    uint8_t FATCount;
    uint16_t RootDirectoryEntries;
    uint16_t Sectors;
    uint8_t MediaDescriptorType;
    uint16_t SectorsPerFAT;
    uint16_t SectorsPerTrack;
    uint16_t Heads;
    uint32_t HiddenSectors;
    uint32_t LargeSectorCount;

    uint8_t DriveNumber;
    uint8_t Reserved;
    uint8_t Signature;
    uint32_t VolumeID;
    uint8_t VolumeLabel[11];
    uint8_t SystemIdentifier[8];
} __attribute__((packed)) FAT_BootSector;

typedef struct {
    bool opened;
    FAT_File public;
    uint8_t buffer[SECTOR_SIZE];
    uint32_t firstCluster;
    uint32_t currentCluster;
    uint32_t currentSectorInCluster;
} FAT_FileData;

typedef struct {
    union {
        FAT_BootSector bootSector;
        uint8_t buffer[SECTOR_SIZE] ;
    } BS;


    FAT_FileData rootDirectory;
    FAT_FileData files[MAX_FILE_HANDLES];

} FAT_Data;

FAT_Data* data;
uint8_t* FAT;

uint32_t dataLBA;

bool FAT_ReadBootSector(DISK* disk) {
    return DISK_ReadSectors(disk, 0, 1, &data->BS.buffer);
}

bool FAT_ReadFAT(DISK* disk) {
    return DISK_ReadSectors(disk, data->BS.bootSector.ReservedSectors, data->BS.bootSector.SectorsPerFAT, FAT);
}

bool FAT_Initialize(DISK* disk) {
    // Allocate FAT data
    data = (FAT_Data*)MEMORY_FAT_ADDR;
    

    // Load bootsector
    if (!FAT_ReadBootSector(disk)) {
        puts("FAT: Error reading boot sector\n");
        return false;
    }

    // TODO: Check if sufficient memory to store the FAT table.

    FAT = (uint8_t*)data + sizeof(FAT_Data);
    // Load FAT
    if (!FAT_ReadFAT(disk)) {
        puts("FAT: Error reading FAT\n");
        return false;
    }

    uint32_t rootDirLBA = data->BS.bootSector.ReservedSectors + data->BS.bootSector.SectorsPerFAT * data->BS.bootSector.FATCount;
    uint32_t rootDirSize = data->BS.bootSector.RootDirectoryEntries * sizeof(FAT_DirectoryEntry);

    data->rootDirectory.opened = true;
    data->rootDirectory.public.handle = ROOT_DIRECTORY_HANDLE;
    data->rootDirectory.public.isDirectory = true;
    data->rootDirectory.public.position = 0;
    data->rootDirectory.public.size = rootDirSize;
    data->rootDirectory.firstCluster = rootDirLBA;
    data->rootDirectory.currentCluster = rootDirLBA;
    data->rootDirectory.currentSectorInCluster = 0;


    if (!DISK_ReadSectors(disk, rootDirLBA, 1, &data->rootDirectory.buffer)) {
        puts("FAT: Error reading root directory\n");
        return false;
    }

    dataLBA = rootDirLBA + (rootDirSize + data->BS.bootSector.BytesPerSector - 1) / data->BS.bootSector.BytesPerSector;

    for (int i = 0; i < MAX_FILE_HANDLES; i++) {
        data->files[i].opened = false;
    }

    return true;
}

uint32_t FAT_ClusterToLBA(uint32_t cluster) {
    return dataLBA + (cluster-2) * data->BS.bootSector.SectorsPerCluster;
}

uint32_t FAT_NextCluster(DISK* disk, uint32_t current) {
    uint32_t unmasked = *(uint16_t*)(FAT+ current * 3 /2);
    if (current % 2 == 0)
        return unmasked & 0xfff;
    else
        return unmasked >> 4;

}

uint32_t FAT_Read(DISK* disk, FAT_File* file, uint32_t count, void* out) {
    FAT_FileData* fd = file->handle == ROOT_DIRECTORY_HANDLE ? 
        &data->rootDirectory : 
        &data->files[file->handle];

    uint8_t* outBuffer = (uint8_t*)out;

    if (!file->isDirectory || (file->isDirectory && file->size > 0))
        count = min(count, file->size - file->position);

    while (count > 0) {
        uint32_t leftInBuffer = SECTOR_SIZE - file->position % SECTOR_SIZE;
        uint32_t take = min(count, leftInBuffer);

        memcpy(outBuffer, fd->buffer + file->position % SECTOR_SIZE, take);
        file->position += take;
        outBuffer += take;

        count -= take;
    
        // Need to load new data into the buffer
        if (take == leftInBuffer) {
            // If it is the root directly it works differently
            if (file->handle == ROOT_DIRECTORY_HANDLE) {
                if(!DISK_ReadSectors(disk, FAT_ClusterToLBA(++fd->currentCluster), 1, fd->buffer)) {
                    puts("FAT: Read error\n");
                    break;
                }
            } else {
                if (++fd->currentSectorInCluster >= data->BS.bootSector.SectorsPerCluster) {
                    fd->currentSectorInCluster = 0;
                    fd->currentCluster = FAT_NextCluster(disk, fd->currentCluster);
                } 

                if (fd->currentCluster >= 0xff8) {
                    file->size = file->position;
                    break;
                }
                
                if(!DISK_ReadSectors(disk, FAT_ClusterToLBA(fd->currentCluster) + fd->currentSectorInCluster, 1, fd->buffer)) {
                    puts("FAT: Read error\n");
                    break;   
                }
            }
        }
    }

    return outBuffer - (uint8_t*)out;
}

bool FAT_ReadDirectoryEntry(DISK* disk, FAT_File* directory, FAT_DirectoryEntry* entryOut) {
    uint32_t readBytes = FAT_Read(disk, directory, sizeof(FAT_DirectoryEntry), entryOut);
    return (readBytes == sizeof(FAT_DirectoryEntry));
}

FAT_File* FAT_OpenEntry(DISK* disk, FAT_DirectoryEntry* entry) {
    // Search for an available file handle
    uint32_t handle = -1;
    for (uint32_t i = 0; i < MAX_FILE_HANDLES; i++) {
        if (!data->files[i].opened)
            handle = i;
    }

    FAT_FileData* fileData = &data->files[handle];

    fileData->firstCluster = entry->startClusterLow | (entry->startClusterHigh << 16);
    fileData->currentCluster = entry->startClusterLow | (entry->startClusterHigh << 16);
    fileData->currentSectorInCluster = 0;
    fileData->public.handle = handle;
    fileData->public.isDirectory = (entry->attributes & FAT_ATTRIBUTE_DIRECTORY) != 0;
    fileData->public.position = 0;
    fileData->public.size = entry->size;

    // Load data into bufffer
    if (!DISK_ReadSectors(disk, FAT_ClusterToLBA(fileData->currentCluster), 1, fileData->buffer)) {
        puts("FAT: Failed to read file into buffer\n");
        return NULL;
    }

    fileData->opened = true;

    return &fileData->public;
}

bool FAT_FindFile(DISK* disk, FAT_File* directory, const char* filename, FAT_DirectoryEntry* entryOut) {

    char convertedFilename[12];
    memset(convertedFilename, ' ', 11);
    convertedFilename[11] = '\0'; 
    
    char* ext = strchr(filename, '.') + 1;

    uint32_t i = 0;
    while (filename[i] != '\0' && (filename + i < ext-1 || ext == NULL)) {
        convertedFilename[i] = toupper(filename[i]);
        i++;
    }

    if (ext != NULL) {
        uint32_t extLen = strlen(ext);
        char* extBeginning = convertedFilename + (11 - extLen);

        for (uint32_t i = 0; i < extLen; i++) {
            extBeginning[i] = toupper(ext[i]);
        }

    }
    
    FAT_DirectoryEntry entry;
    while(FAT_ReadDirectoryEntry(disk, directory, &entry)) {
        if (memcmp(entry.filename, convertedFilename, 11) == 0) {
            *entryOut = entry;
            return true;
        }
    }

    puts(filename);
    puts(convertedFilename);

    return false;
}

void FAT_Close(FAT_File* file) {
    if (file->handle == ROOT_DIRECTORY_HANDLE) {
        file->position = 0;
        data->rootDirectory.currentCluster = data->rootDirectory.firstCluster;
    } else {
        data->files[file->handle].opened = false;
    }
}

FAT_File* FAT_Open(DISK* disk, const char* path) {
    char name[MAX_PATH_SIZE];
    
    FAT_File* current = &data->rootDirectory.public;

    if (*path == '/') 
        path++;

    while (*path) {
        bool isLast = false;
        char* delim = strchr(path, '/');
        if (delim) {

            memcpy(name, path, delim - path);
            name[delim - path] = '\0';
            path = delim + 1;

        } else {
            isLast = true;
            uint32_t len = strlen(path);
            memcpy(name, path, len);
            name[len] = '\0';
            path += len;
        }

        FAT_DirectoryEntry entry;
        if (FAT_FindFile(disk, current, name, &entry)) {
            // Close previous file

            if (!isLast && entry.attributes & FAT_ATTRIBUTE_DIRECTORY == 0) {
                puts("FAT: File is not a directory");
                return NULL;
            }

            FAT_Close(current);
            current = FAT_OpenEntry(disk, &entry);

        } else {
            FAT_Close(current);
            puts("FAT: Couldn't find file\n");
            return NULL;
        }
    }

    return current;
}