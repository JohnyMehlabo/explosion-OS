#include "memory.h"

void* memset(void* dest, uint32_t c, size_t count) {
    for (uint32_t i = 0; i < count; i++) {
        ((uint8_t*)dest)[i] = c;
    }

    return dest;
}

void* memcpy(void* dest, const void* src, size_t count) {
    
    for (size_t i = 0; i < count; i++) {
        ((uint8_t*)dest)[i] = ((uint8_t*)src)[i];
    }

    return dest;
}

int32_t memcmp(const void* buf1, const void* buf2, size_t count) {
    uint8_t* u8Buf1 = (uint8_t*) buf1;
    uint8_t* u8Buf2 = (uint8_t*) buf2;
    for (uint32_t i = 0; i < count; i++) {
        if (u8Buf1[i] != u8Buf2[i])
            return u8Buf1[i] - u8Buf2[i];
    }

    return 0;
}
