#include "stdio.h"
#include "drivers/display.h"
#include "stdarg.h"

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

int screenX = 0, screenY = 0;

uint8_t defaultColor = 0x1f;

void clrscr() {
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        for (int y = 0; y < SCREEN_HEIGHT; y++) {
            Display_PutCharWithColor(x, y, '\0', defaultColor);
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
        case '\b':
            if (screenX > 0) {
                screenX--;
                Display_PutChar(screenX, screenY, '\0');
            }
            break;
        default:
            Display_PutChar(screenX, screenY, c);
            screenX++;
            break;
    }
}

void puts(const char* str) {
    while(*str) {
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

const char hexCharacters[] = "0123456789ABCDEF";

// Putd hexadecimal
void putd_h(uint32_t number) {
    char buffer[32];
    int pos = 0;
    do {
        buffer[pos++] = hexCharacters[number % 16];
        number /= 16;
    } while (number > 0);

    while (--pos >= 0) {
        putc(buffer[pos]);
    }
}

enum PrintfState {
    DEFAULT,
    FORMAT_SPECIFIER,
};

void printf(const uint8_t* format, ...) {
    va_list argptr;
    va_start(argptr, format);
    
    enum PrintfState state = DEFAULT;

    while (*format) {
        switch (state)
        {
        case DEFAULT:
            switch (*format) {
                case '%':
                    state = FORMAT_SPECIFIER;
                    break;
                default:
                    putc(*format);
                    break;
            }
            break;
        case FORMAT_SPECIFIER:
            switch (*format) {
                case 'd':
                    unsigned int d = va_arg(argptr, unsigned int);
                    putd(d);
                    break;
                case 'x':
                    unsigned int x = va_arg(argptr, unsigned int);
                    putd_h(x);
                    break;
                case 's':
                    const char* s = va_arg(argptr, const char*);
                    puts(s);
                    break;
                case 'c':
                    unsigned int c = va_arg(argptr, unsigned int);
                    putc(c);
                    break;
                case '%':
                    putc('%');
                    break;
            }
            state = DEFAULT;
            break;
            
        }
        format++;
    }
    va_end(argptr);
}
