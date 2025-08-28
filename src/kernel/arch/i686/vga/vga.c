#include "vga.h"
#include "vga_modes.h"
#include "../io.h"

void i686_VGA_SetMode(VGAMode mode) {
    const VGARegisters* registers = i686_VGA_GetRegisters(mode);

    // Load Miscellaneous Output Register
    i686_outb(0x3c2, registers->miscellaneousOutputRegister);

    // Load sequencer registers
    for (uint8_t i = 0; i < 5; i++) {
        i686_outb(0x3c4, i);
        i686_outb(0x3c4+1, registers->sequencer[i]);
    }

    // Load CRT controller registers
    i686_outb(0x3d4, 0x03);
    i686_outb(0x3d4+1, i686_inb(0x3d4+1) | 0x80);
    i686_outb(0x3d4, 0x11);
    i686_outb(0x3d4+1, i686_inb(0x3d4+1) & ~0x80);
    for (uint8_t i = 0; i < 25; i++) {
        i686_outb(0x3d4, i);
        i686_outb(0x3d4+1, registers->crtController[i]);
    }

    // Load graphics controller registers
    for (uint8_t i = 0; i < 9; i++) {
        i686_outb(0x3ce, i);
        i686_outb(0x3ce +1, registers->graphicsController[i]);
    }

    // Load attribute controller registers
    for (uint8_t i = 0; i < 21; i++) {
        i686_inb(0x3da);
        i686_outb(0x3c0, i);
        i686_outb(0x3c0, registers->attributeController[i]);
    }

    // Load some example color
    i686_outb(0x3C8, 1);        // Set palette write index to 1
    i686_outb(0x3C9, 63);       // Red component (max)
    i686_outb(0x3C9, 0);        // Green component
    i686_outb(0x3C9, 0);

    i686_inb(0x3da);
    i686_outb(0x3c0, 0x20);
}

void VGA_LoadFont(const uint8_t* font) {
    // Save registers
    uint8_t seq2, seq4, gc4, gc5, gc6;

    // Save original state
    i686_outb(0x3C4, 0x02);
    seq2 = i686_inb(0x3C5);
    i686_outb(0x3C4, 0x04);
    seq4 = i686_inb(0x3C5);
    i686_outb(0x3CE, 0x04);
    gc4 = i686_inb(0x3CF);
    i686_outb(0x3CE, 0x05);
    gc5 = i686_inb(0x3CF);
    i686_outb(0x3CE, 0x06);
    gc6 = i686_inb(0x3CF);

    // Set sequencer to access plane 2
    i686_outb(0x3C4, 0x02);
    i686_outb(0x3C5, 0x04); // Plane 2

    // Set memory mode to sequential
    i686_outb(0x3C4, 0x04);
    i686_outb(0x3C5, 0x07); // Enable sequential access

    // Graphics controller: write to plane 2
    i686_outb(0x3CE, 0x04);
    i686_outb(0x3CF, 0x02); // Plane 2
    i686_outb(0x3CE, 0x05);
    i686_outb(0x3CF, 0x00); // Write mode 0
    i686_outb(0x3CE, 0x06);
    i686_outb(0x3CF, 0x00); // Map to 0xA0000

    // Write the font to plane 2 memory at 0xc00a0000
    volatile uint8_t* font_mem = (uint8_t*)0xc00a0000;
    for (int ch = 0; ch < 256; ++ch) {
        for (int row = 0; row < 16; ++row) {
            font_mem[ch * 32 + row] = font ? font[ch * 16 + row] : 0;
        }
    }

    // Restore registers
    i686_outb(0x3C4, 0x02);
    i686_outb(0x3C5, seq2);
    i686_outb(0x3C4, 0x04);
    i686_outb(0x3C5, seq4);
    i686_outb(0x3CE, 0x04);
    i686_outb(0x3CF, gc4);
    i686_outb(0x3CE, 0x05);
    i686_outb(0x3CF, gc5);
    i686_outb(0x3CE, 0x06);
    i686_outb(0x3CF, gc6);
}

void VGA_LoadPalette(const VGAPalette* palette, uint8_t start) {
    i686_outb(0x3C8, start); // Start at palette start index
    for (int i = 0; i < palette->count; ++i) {
        i686_outb(0x3C9, palette->colors[i*3]); // We muliply by 3 because each color uses 3 bytes
        i686_outb(0x3C9, palette->colors[i*3 +1]);
        i686_outb(0x3C9, palette->colors[i*3 +2]);
    }
}

