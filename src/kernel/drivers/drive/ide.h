#pragma once
#include "stdint.h"

void IDE_Init();
uint8_t IDE_Access(uint8_t driveNumber, uint8_t direction, uint32_t lba, uint8_t sectorCount, uint8_t* outputBuffer);