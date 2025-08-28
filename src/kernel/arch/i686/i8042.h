#pragma once
#include "stdbool.h"
#include "stdint.h"

bool i686_i8042_ReadByte(uint8_t* out);
bool i686_i8042_WriteByte(uint8_t value);