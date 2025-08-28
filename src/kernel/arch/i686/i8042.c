#include "i8042.h"
#include "io.h"

#define I8042_DATA_PORT 0x60
#define I8042_STATUS_REGISTER 0x64
#define I8042_COMMAND_REGISTER 0x64

bool i686_i8042_WaitOutputBufferFull() {
    for (uint32_t i = 0; i < 100000; i++) {
        if (i686_inb(I8042_STATUS_REGISTER) & 0b00000001)
            return true;
    }
    return false;
}

bool i686_i8042_WaitInputBufferEmpty() {
    for (uint32_t i = 0; i < 100000; i++) {
        if (!(i686_inb(I8042_STATUS_REGISTER) & 0b00000010))
            return true;
    }
    return false;
}

bool i686_i8042_ReadByte(uint8_t* out) {
    if (!i686_i8042_WaitOutputBufferFull()) return false;
    *out = i686_inb(I8042_DATA_PORT);
    return true;
}

bool i686_i8042_WriteByte(uint8_t value) {
    if (!i686_i8042_WaitInputBufferEmpty()) return false;
    i686_outb(I8042_DATA_PORT, value);
    return true;
}