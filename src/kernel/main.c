#include "stdint.h"
#include "memory.h"
#include "stdio.h"
#include "arch/i686/gdt.h"
#include "arch/i686/idt.h"
#include "arch/i686/isr.h"
#include "arch/i686/irq.h"
#include "arch/i686/io.h"
#include "drivers/ps2.h"
#include "drivers/display.h"
#include "drivers/keyboard.h"
#include "drivers/pci.h"
#include "drivers/drive/ide.h"
#include "drivers/net/rtl8139.h"
#include "net/arp.h"
#include "shell.h"

extern uint8_t __bss_start;
extern uint8_t __bss_end;

void print_test() {
    puts("Hello world!\n");
}

void enableFPU() {
    uint32_t cr0;

    // Read CR0
    __asm__ volatile ("mov %%cr0, %0" : "=r"(cr0));

    // Clear EM (bit 2), set MP (bit 1)
    cr0 &= ~(1 << 2); // Clear EM
    cr0 |=  (1 << 1); // Set MP

    // Write back to CR0
    __asm__ volatile ("mov %0, %%cr0" :: "r"(cr0));

    // Initialize FPU
    __asm__ volatile ("fninit");
}

void kstart(uint16_t bootDrive) {
    memset(&__bss_start, 0, (&__bss_end) - (&__bss_start));

    clrscr();
    
    i686_GDT_Initialize();       
    i686_IDT_Initialize();
    
    i686_ISR_Initialize();

    i686_IRQ_Initialize();
    i686_IRQ_SetMask(0);   
    i686_EnableInterrupts();

    enableFPU();

    PS2_Initialize();
    Display_Initialize();

    PCI_EnumerateDevices();
    IDE_Init();
    RTL8139_Init();

    ARP_SendRequest(0x0101a8c0);
    
    shell();

end:
    for(;;);
}
