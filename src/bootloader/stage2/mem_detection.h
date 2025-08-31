#pragma once
#include "stdint.h"
#include "x86.h"

typedef struct {
    uint32_t baseLo;
    uint32_t baseHi;
    uint32_t lengthLo;
    uint32_t lengthHi;
    uint32_t type;
    uint32_t ACPI;
} SMAPEntry;

uint16_t MemDect_DetectMemory(void* outAddr);