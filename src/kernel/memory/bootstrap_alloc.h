#pragma once
#include "stdint.h"
#include "stddef.h"

void BootAlloc_Init();
void* bmalloc(size_t size);
void BootAlloc_FreeAll();