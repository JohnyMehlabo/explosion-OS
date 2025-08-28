#include "stdint.h"
#pragma once

void i686_IDT_Initialize();
void i686_IDT_SetGate(uint8_t vector, void* offset, uint16_t segmentSelector, uint8_t flags);
void i686_IDT_EnableGate(uint8_t vector);
void i686_IDT_DisableGate(uint8_t vector);