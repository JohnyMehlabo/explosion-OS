#include "8259.h"
#include "../io.h"

#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xa0
#define PIC2_DATA 0xa1

#define PIC_EOI 0x20

void i686_PIC_SendEOI(uint8_t irq) {
    if (irq >= 8)
        i686_outb(PIC2_COMMAND, PIC_EOI);
    
    i686_outb(PIC1_COMMAND, PIC_EOI);
}

#define ICW1_ICW4	0x01		/* Indicates that ICW4 will be present */
#define ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	0x10		/* Initialization - required! */

#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	0x10		/* Special fully nested (not) */

/*
arguments:
	offset1 - vector offset for master PIC
		vectors on the master become offset1..offset1+7
	offset2 - same for slave PIC: offset2..offset2+7
*/
void i686_PIC_Remap(uint8_t offset1, uint8_t offset2)
{
	i686_outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);  // starts the initialization sequence (in cascade mode)
	i686_IOWait();
	i686_outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    i686_IOWait();
	i686_outb(PIC1_DATA, offset1);                 // ICW2: Master PIC vector offset
	i686_IOWait();
    i686_outb(PIC2_DATA, offset2);                 // ICW2: Slave PIC vector offset
	i686_IOWait();
    i686_outb(PIC1_DATA, 4);                       // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	i686_IOWait();
    i686_outb(PIC2_DATA, 2);                       // ICW3: tell Slave PIC its cascade identity (0000 0010)
	i686_IOWait();

	i686_outb(PIC1_DATA, ICW4_8086);               // ICW4: have the PICs use 8086 mode (and not 8080 mode)
	i686_IOWait();
    i686_outb(PIC2_DATA, ICW4_8086);
    i686_IOWait();

	// Unmask both PICs.
	i686_outb(PIC1_DATA, 0);
	i686_outb(PIC2_DATA, 0);
}

void i686_PIC_Disable() {
    i686_outb(PIC1_DATA, 0xff);
    i686_outb(PIC2_DATA, 0xff);
}

void i686_PIC_SetMask(uint8_t line) {
    uint16_t dataPort = PIC1_DATA;
    
    if (line >= 8) {
        dataPort = PIC2_DATA;
        line -= 8;
    }

    uint8_t data = i686_inb(dataPort) | (1 << line);
    i686_outb(dataPort, data);
}

void i686_PIC_ClearMask(uint8_t line) {
    uint16_t dataPort = PIC1_DATA;
    
    if (line >= 8) {
        dataPort = PIC2_DATA;
        line -= 8;
    }

    uint8_t data = i686_inb(dataPort) & ~(1 << line);
    i686_outb(dataPort, data);
}