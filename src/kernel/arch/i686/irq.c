#include "irq.h"
#include "stdint.h"
#include "stddef.h"
#include "stdio.h"
#include "pic/8259.h"
#include "isr.h"

#define MAX_IRQ_HANDLERS 0x10
#define PIC_REMAP_OFFSET 0x20

void i686_IRQ_CommonHandler(StackData* data);

void i686_IRQ_Initialize() {
    i686_PIC_Remap(PIC_REMAP_OFFSET, PIC_REMAP_OFFSET+8);

    for (uint8_t i = PIC_REMAP_OFFSET; i < PIC_REMAP_OFFSET+MAX_IRQ_HANDLERS; i++) {
        i686_ISR_SetHandler(i, i686_IRQ_CommonHandler);
    }
}

static IRQHandler handlers[MAX_IRQ_HANDLERS];

void i686_IRQ_CommonHandler(StackData* data) {
    if (handlers[data->interruptVector-PIC_REMAP_OFFSET] != NULL) {
        handlers[data->interruptVector-PIC_REMAP_OFFSET](data);
    } else {
        puts("Got IRQ: ");
        putd(data->interruptVector-PIC_REMAP_OFFSET);
        putc('\n');
    }
    
    i686_PIC_SendEOI(data->interruptVector-PIC_REMAP_OFFSET);
}

void i686_IRQ_SetHandler(uint8_t line, IRQHandler handler) {
    handlers[line] = handler;
}

void i686_IRQ_SetMask(uint8_t line) {
    i686_PIC_SetMask(line);
}

void i686_IRQ_ClearMask(uint8_t line) {
    i686_PIC_ClearMask(line);
}