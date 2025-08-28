#pragma once
#include "stdint.h"

typedef enum {
    VGA_80x25_TEXT_MODE,
    VGA_320x200_LINEAR_256_COLOR_MODE,
} VGAMode;

typedef struct {
    const uint8_t* colors;
    uint8_t count;
} VGAPalette;

void i686_VGA_SetMode(VGAMode mode);
void VGA_LoadFont(const uint8_t* font);
void VGA_LoadPalette(const VGAPalette* palette, uint8_t start);