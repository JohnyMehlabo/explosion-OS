#include "raytracer.h"
#include "memory.h"
#include "vector.h"
#include "renderer.h"
#include "math.h"
#include "drivers/keyboard.h"
#include "drivers/display.h"

#define IMAGE_WIDTH 320
#define IMAGE_HEIGHT 200
#define IMAGE_ASPECT_RATIO (float)IMAGE_WIDTH/IMAGE_HEIGHT

static void plotPixel(uint32_t x, uint32_t y, const Vector3* color) {
    uint8_t r = round(color->x * 5);
    uint8_t g = round(color->y * 5);
    uint8_t b = round(color->z * 5);

    Display_PlotPixel(x, y, 16 + 36*r + 6*g + b);
}

void raytracer() {
    Display_EnterColorMode();

    for (uint32_t y = 0; y < IMAGE_HEIGHT; y++) {
        for (uint32_t x = 0; x < IMAGE_WIDTH; x++) {
            Vector3 color;
            float normalizedX = (((float)x/IMAGE_WIDTH)*2-1) * IMAGE_ASPECT_RATIO;
            float normalizedY = ((float)y/IMAGE_HEIGHT)*2-1;
            Renderer_PerPixel(normalizedX, normalizedY, &color);
            plotPixel(x, y, &color);;
        }   
    }

    while (!Keyboard_IsKeyDown(KEY_BACKSPACE)) {    } // Halt until backspace key is pressed, then return
    Display_EnterTextMode();
}