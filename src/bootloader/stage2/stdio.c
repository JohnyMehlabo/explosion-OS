#include "stdio.h"

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

volatile uint8_t* screenBuffer = (uint8_t*)0xB8000;
int screenX = 0, screenY = 0;

uint8_t defaultColor = 0x1f;

void clrscr() {
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        for (int y = 0; y < SCREEN_HEIGHT; y++) {
            ((uint16_t*)screenBuffer)[y * 80 + x] = (uint16_t)'\0' | (defaultColor << 8);
        }
    }

    screenX = 0;
    screenY = 0;
}

void putc(char c) {
    switch (c) {
        case '\n':
            screenY++;
            screenX = 0;
            break;
        default:
            screenBuffer[(screenY * 80 + screenX) * 2] = c;
            screenX++;
            break;
    }
    
}

void puts(const char* str) {
    while(*str)
    {
        putc(*str);
        str++;
    }
}

void putd(uint32_t number) {
    char buffer[32];
    int pos = 0;
    do {
        buffer[pos++] = number % 10 + '0';
        number /= 10;
    } while (number > 0);

    while (--pos >= 0) {
        putc(buffer[pos]);
    }
}