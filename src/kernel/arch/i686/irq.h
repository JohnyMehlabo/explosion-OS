#pragma once
#include "isr.h"

typedef void (*IRQHandler)(StackData* data);

void i686_IRQ_Initialize();
void i686_IRQ_SetHandler(uint8_t line, IRQHandler handler);
void i686_IRQ_SetMask(uint8_t line);
void i686_IRQ_ClearMask(uint8_t line);