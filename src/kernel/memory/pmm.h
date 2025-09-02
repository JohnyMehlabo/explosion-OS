#pragma once
#include "stdint.h"
#include "boot_info.h"

void PMM_Init(const SMAPEntry* smapEntries, uint32_t entryCount);
uint32_t PMM_GetFrames(uint32_t count);
void PMM_FreeFrames(uint32_t address, uint32_t count);