#include "shell.h"
#include "drivers/keyboard.h"
#include "stdio.h"
#include "string.h"
#include "memory.h"
#include "apps/raytracer/raytracer.h"
#include "drivers/display.h"

#define INPUT_BUFFER_SIZE 256

void shell() {
    uint8_t inputBuffer[INPUT_BUFFER_SIZE];
    inputBuffer[INPUT_BUFFER_SIZE-1] = '\0';
    
    while (true) { // Main fetch loop
        putc('>');
        putc(' ');
        uint8_t* inputBufferPtr = inputBuffer; // Pointer to current character
        bool newLine = false;
        while (!newLine) { // Main character read loop
            while (!Keyboard_CharacterQueueEmpty())
            {
                uint8_t character = Keyboard_GetCharacter();

                if (character == '\b') {
                    if (inputBufferPtr > inputBuffer) {
                        *(--inputBufferPtr) = '\0';
                        putc(character);
                    }
                }
                else if (character == '\n') {
                    *(inputBufferPtr) = '\0';
                    newLine = true;
                    putc(character);
                    break;
                } else {
                    *(inputBufferPtr++) = character;
                    putc(character);
                }
            }
        }

        if (strcmp(inputBuffer, "info") == 0) {
            puts("Welcome to ExplosionOS!\n");
        } else if (strcmp(inputBuffer, "graphics") == 0) {
            Display_EnterColorMode();
            for (uint32_t y = 0; y < 200; y++) {
                for (uint32_t x = 0; x < 320; x++) {
                    Display_PlotPixel(x, y, 16 + 36*5 + 6*5 + 0);
                }
            }

            // Halt in graphics mode until enter key is pressed
            while (!Keyboard_IsKeyDown(KEY_BACKSPACE)) {    }
            Display_EnterTextMode();
            Keyboard_FlushCharacterQueue();
            clrscr();
        } else if (strcmp(inputBuffer, "clear") == 0) {
            clrscr();
        } else if (strcmp(inputBuffer, "raytracer") == 0) {
            raytracer();
            Keyboard_FlushCharacterQueue();
            clrscr();
        } else if (strcmp(inputBuffer, "CARLOS") == 0) {
            Display_EnterColorMode();
            for (uint32_t y = 0; y < 200; y++) {
                for (uint32_t x = 0; x < 320; x++) {
                    Display_PlotPixel(x, y, 16 + 36*(y%5) + 6*(x%5) + (x/5)%5);
                }
            }

            // Halt in graphics mode until enter key is pressed
            while (!Keyboard_IsKeyDown(KEY_BACKSPACE)) {    }
            Display_EnterTextMode();
            Keyboard_FlushCharacterQueue();
            clrscr();
            puts("CARLOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOS\n");
        }
    }

}