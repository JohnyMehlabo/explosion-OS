#pragma once
#include "stdint.h"
#include "stdbool.h"

void VMM_Init();
void* VMM_ReservePages(uint32_t count);
void* VMM_AllocatePages(uint32_t count, bool contiguous);
void VMM_FreePages(uint8_t* addr, uint32_t count);
void* VMM_GetPhysicalMapping(void* addr);