#include "gdt.h"
#include "stdint.h"
#include "utils/assembly.h"

typedef struct {
    uint16_t limit;
    uint16_t base;
    uint8_t baseMe;
    uint8_t accessByte;
    uint8_t flagsLimitHi;
    uint8_t baseHi;
}  __attribute__((packed)) GDTEntry;

typedef struct {
    uint16_t size;
    const GDTEntry* offset;
}  __attribute__((packed)) GDTDescriptor;

static const GDTEntry gdt[] = {
    {
        .limit = 0x0000,
        .base = 0x0000,
        .baseMe = 0x00,
        .accessByte = 0x00,
        .flagsLimitHi = 0x00,
        .baseHi = 0x0000 
    },
    {
        .limit = 0xffff,
        .base = 0x0000,
        .baseMe = 0x00,
        .accessByte = 0b10011010,
        .flagsLimitHi = 0b11001111,
        .baseHi = 0x0000 
    },
    {
        .limit = 0xffff,
        .base = 0x0000,
        .baseMe = 0x00,
        .accessByte = 0b10010010,
        .flagsLimitHi = 0b11001111,
        .baseHi = 0x0000 
    },
};

static const GDTDescriptor gdtr = {
    .size = sizeof(gdt) - 1,
    .offset = gdt
};

extern void ASSEMBLY loadGDT(const GDTDescriptor* gdtr);

void i686_GDT_Initialize() {
    loadGDT(&gdtr);
}