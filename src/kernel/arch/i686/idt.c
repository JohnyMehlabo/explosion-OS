#include "idt.h"
#include "io.h"
#include "utils/assembly.h"
#include "utils/flag.h"

#define MAX_IDT_ENTRIES 256

typedef struct {
    uint16_t offsetLo;
    uint16_t segmentSelector;
    uint8_t reserved;
    uint8_t flags;
    uint16_t offsetHi;
} __attribute__((packed)) GateEntry;

typedef struct {
    uint16_t size;
    const GateEntry* offset;
}  __attribute__((packed)) IDTDescriptor;

static GateEntry idt[MAX_IDT_ENTRIES];

static IDTDescriptor idtr = {
    .size = sizeof(idt) -1,
    .offset = idt
};

extern void ASSEMBLY loadIDT(IDTDescriptor* idtr);

void i686_IDT_Initialize() {
    loadIDT(&idtr);
}

void i686_IDT_SetGate(uint8_t vector, void* offset, uint16_t segmentSelector, uint8_t flags) {
    idt[vector].flags = flags;
    idt[vector].segmentSelector = segmentSelector;
    idt[vector].offsetLo = (uint32_t)offset & 0xFFFF;
    idt[vector].offsetHi = (uint32_t)offset >> 16;
}

void i686_IDT_EnableGate(uint8_t vector) {
    idt[vector].flags = FLAG_SET(idt[vector].flags, 7);
}

void i686_IDT_DisableGate(uint8_t vector) {
    idt[vector].flags = FLAG_UNSET(idt[vector].flags, 7);
}