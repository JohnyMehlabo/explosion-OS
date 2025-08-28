#pragma once
#include "stdint.h"
#include "stddef.h"

void* memset(void* dest, uint32_t c, size_t count);
void* memcpy(void* dest, const void* src, size_t count);
int32_t memcmp(const void* buf1, const void* buf2, size_t count);

