#include "isr.h"
#include "pic/8259.h"
#include "io.h"
#include "stddef.h"
#include "isr_gen.h"
#include "../../stdio.h"

#define MAX_ISR_HANDLERS 256
static ISRHandler handlers[MAX_ISR_HANDLERS];
#define MAX_EXCEPTION_HANDLERS 32
static ISRHandler exceptionHandlers[MAX_EXCEPTION_HANDLERS];

void i686_ISR_Initialize() {
    setupHandlers();
    for (uint32_t i = 0; i < MAX_ISR_HANDLERS; i++) {
        i686_IDT_EnableGate(i);
    }
}

void i686_ISR_SetHandler(uint8_t vector, ISRHandler handler) {
    handlers[vector] = handler;
}

void i686_ISR_SetExceptionHandler(uint8_t vector, ISRHandler handler) {
    exceptionHandlers[vector] = handler;
}

// TODO: Complete this
const char* exceptionMessages[] = {"Division by zero", "Debug", "Non-maskable interrupt", "Breakpoint", "Overflow", "Bound Range Exceeded", "Invalid Opcode", "Device Not Available", "Double Fault", "Coprocessor Segment Overrun", "Invalid TSS", "Segment Not Present", "Stack-Segment Fault", "General Protection Fault", "Page Fault", "", "x87 Floating-Point Exception", "Alignment Check", "Machine Check", "SIMD Floating-Point Exception", "Virtualization Exception", "Control Protection Exception", "", "", "", "", "", "", "Hypervisor Injection Exception", "VMM Communication Exception", "Security Exception", ""};

void i686_ISR_CommonHandler(StackData* data) {
    if (data->interruptVector < 0x20) {
        // Check if an exception handler is present
        if (exceptionHandlers[data->interruptVector] != NULL) {
            exceptionHandlers[data->interruptVector](data);
            return;
        }

        printf("Unhandled exception: %s\n", exceptionMessages[data->interruptVector]);
        if (data->interruptVector == 14) {
            printf("0x%x\n", data->cr2);
        }

        i686_Halt();
    }

    if (handlers[data->interruptVector] != NULL) {
        handlers[data->interruptVector](data);
    } else {
        puts("Unhandled interrupt: ");

        putd(data->interruptVector);
        putc('\n');
    }
}