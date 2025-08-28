#pragma once
#include "utils/assembly.h"
#include "stdint.h"

extern void ASSEMBLY i686_EnableInterrupts();
extern void ASSEMBLY i686_DisableInterrupts();
extern void ASSEMBLY i686_Halt();
extern uint8_t ASSEMBLY i686_inb(uint16_t port);
extern void ASSEMBLY i686_outb(uint16_t port, uint8_t value);
extern uint16_t ASSEMBLY i686_inw(uint16_t port);
extern void ASSEMBLY i686_outw(uint16_t port, uint16_t value);
extern uint32_t ASSEMBLY i686_inl(uint16_t port);
extern void ASSEMBLY i686_outl(uint16_t port, uint32_t value);
void i686_IOWait();
