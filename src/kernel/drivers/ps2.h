#pragma once
#include "stdbool.h"
#include "stdint.h"

void PS2_Initialize();
bool PS2_EnqueueCommand(uint8_t* bytes, uint8_t len);