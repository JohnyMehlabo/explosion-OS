#pragma once
#include "stdint.h"

typedef struct {
    uint32_t baseLo;
    uint32_t baseHi;
    uint32_t lengthLo;
    uint32_t lengthHi;
    uint32_t type;
    uint32_t ACPI;
} SMAPEntry;

typedef struct {
    uint16_t bootDrive;
    uint32_t smapEntryCount;
    SMAPEntry* smap;    
} BootInfo;