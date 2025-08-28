#include "display.h"
#include "arch/i686/vga/vga.h"
#include "default_font.h"
#include "memory.h"

static const uint8_t textPaletteData[16][3] = {
    {0, 0, 0},       // 0: black
    {0, 0, 42},      // 1: blue
    {0, 42, 0},      // 2: green
    {0, 42, 42},     // 3: cyan
    {42, 0, 0},      // 4: red
    {42, 0, 42},     // 5: magenta
    {42, 21, 0},     // 6: brown/yellow
    {42, 42, 42},    // 7: light gray
    {21, 21, 21},    // 8: dark gray
    {21, 21, 63},    // 9: bright blue
    {21, 63, 21},    // 10: bright green
    {21, 63, 63},    // 11: bright cyan
    {63, 21, 21},    // 12: bright red
    {63, 21, 63},    // 13: bright magenta
    {63, 63, 21},    // 14: yellow
    {63, 63, 63},    // 15: white
};
static VGAPalette textPalette = {
    .colors = (const uint8_t*)textPaletteData,
    .count = 16
};
static const uint8_t* currentFont = defaultFont;

static volatile uint8_t* textBuffer = (uint8_t*)0xc00b8000;

typedef enum {
    DISPLAY_MODE_TEXT,
    DISPLAY_MODE_COLOR
} DisplayMode;

static DisplayMode currentMode = DISPLAY_MODE_TEXT; // Default to text mode
static uint32_t width = 80;
static uint32_t height = 25;

void Display_EnterTextMode() {
    i686_VGA_SetMode(VGA_80x25_TEXT_MODE); // Set registers
    memset((void*)textBuffer, 0x00, 0x10000); // Clear the text buffer
    VGA_LoadFont(defaultFont); // Load font (0xc00a0000 has probably been overwritten)
    VGA_LoadPalette(&textPalette, 0); // Load the default text palette

    currentMode = DISPLAY_MODE_TEXT;
    width = 80;
    height = 25;
}

void Display_PutChar(uint32_t x, uint32_t y, uint8_t character) {
    textBuffer[(y*width + x) * 2] = character;
}

void Display_PutCharWithColor(uint32_t x, uint32_t y, uint8_t character, uint8_t color) {
    ((uint16_t*)textBuffer)[y*width + x] = (uint16_t)character | (color << 8);
}

static uint8_t colorCubePaletteData[216][3];
VGAPalette colorCubePalette = {
    .colors = (const uint8_t*)colorCubePaletteData,
    .count = 216
};
static void generateColorCubePalette() {

    for (uint8_t r = 0; r < 6; ++r) {
        for (uint8_t g = 0; g < 6; ++g) {
            for (uint8_t b = 0; b < 6; ++b) {
                uint8_t index = r*36 + g*6 + b;
                // Map r, g, b from [0,5] to [0,63]
                colorCubePaletteData[index][0] = r * 63 / 5;
                colorCubePaletteData[index][1] = g * 63 / 5;
                colorCubePaletteData[index][2] = b * 63 / 5;
            }
        }
    }
}

static volatile uint8_t* pixelBuffer = (uint8_t*)0xc00a0000;

void Display_EnterColorMode() {
    i686_VGA_SetMode(VGA_320x200_LINEAR_256_COLOR_MODE);   
    VGA_LoadPalette(&colorCubePalette, 16);
    memset((void*)pixelBuffer, 0x00, 320*200);

    currentMode = DISPLAY_MODE_COLOR;
    width = 320;
    height = 200;
}

void Display_PlotPixel(uint32_t x, uint32_t y, uint8_t color) {
    pixelBuffer[y*width + x] = color;
}

void Display_Initialize() {
    // Initialize data for our color cube palette
    generateColorCubePalette();
}