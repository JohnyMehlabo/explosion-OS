#include "isr.h"
#include "pic/8259.h"
#include "io.h"
#include "stddef.h"
#include "isr_gen.h"
#include "../../stdio.h"

#define MAX_ISR_HANDLERS 256
static ISRHandler handlers[MAX_ISR_HANDLERS];

void i686_ISR_Initialize() {
    setupHandlers();
    for (uint32_t i = 0; i < MAX_ISR_HANDLERS; i++) {
        i686_IDT_EnableGate(i);
    }
}

void i686_ISR_SetHandler(uint8_t vector, ISRHandler handler) {
    handlers[vector] = handler;
}

// TODO: Complete this
const char* exceptionMessages[] = {"Division by zero", "Debug", "Non-maskable interrupt", "Breakpoint", "Overflow", "Bound Range Exceeded", "Invalid Opcode", "Device Not Available", "Double Fault", "Coprocessor Segment Overrun", "Invalid TSS", "Segment Not Present", "Stack-Segment Fault", "General Protection Fault", "Page Fault", "", "x87 Floating-Point Exception", "Alignment Check", "Machine Check", "SIMD Floating-Point Exception", "Virtualization Exception", "Control Protection Exception", "", "", "", "", "", "", "Hypervisor Injection Exception", "VMM Communication Exception", "Security Exception", ""};

void i686_ISR_CommonHandler(StackData* data) {
    if (data->interruptVector < 0x20) {
        puts("Unhandled exception: ");
        puts(exceptionMessages[data->interruptVector]);
        putc('\n');
        i686_Halt();
    }

    if (handlers[data->interruptVector] != NULL) {
        handlers[data->interruptVector](data);
    } else {
        puts("Unhandler interrupt: ");

        putd(data->interruptVector);
        putc('\n');
    }
}