#pragma once
#include "stdint.h"

void Display_Initialize();

void Display_EnterTextMode();
void Display_PutChar(uint32_t x, uint32_t y, uint8_t character);
void Display_PutCharWithColor(uint32_t x, uint32_t y, uint8_t character, uint8_t color);

void Display_EnterColorMode();
void Display_PlotPixel(uint32_t x, uint32_t y, uint8_t color);