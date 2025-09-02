#include "bootstrap_alloc.h"
#include "stdio.h"
#include "memdefs.h"

extern uint8_t __end;

uint8_t* currentPtr;
uint8_t* maxPtr;


void BootAlloc_Init() {
    currentPtr = &__end;
    maxPtr = &__end + BOOTSTRAP_MEM_SIZE;
}

void* bmalloc(size_t size) {
    uint8_t* ptr = currentPtr;
    currentPtr += size;
    if (currentPtr >= maxPtr) {
        return NULL;
    }
    return ptr;
}

void BootAlloc_FreeAll() {
    currentPtr = &__end;
}


