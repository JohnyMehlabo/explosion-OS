#pragma once
#include "stdint.h"

void i686_PIC_SendEOI(uint8_t irq);
void i686_PIC_Remap(uint8_t offset1, uint8_t offset2);
void i686_PIC_Disable();
void i686_PIC_SetMask(uint8_t line);
void i686_PIC_ClearMask(uint8_t line);
