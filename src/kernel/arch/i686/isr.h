#pragma once
#include "stdint.h"

typedef struct {
    uint32_t edi, esi, ebp, useless, ebx, edx, ecx, eax;
    uint32_t interruptVector;
    uint32_t errorCode;
    uint32_t eip, cs, eflags, esp, ss;
} __attribute__((packed)) StackData;

typedef void (*ISRHandler)(StackData* data);

void i686_ISR_Initialize();
void i686_ISR_SetHandler(uint8_t vector, ISRHandler handler);