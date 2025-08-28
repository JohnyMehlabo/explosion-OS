#include "keycode.h"

const uint8_t scancodeMapping[256] = { 

    [0x02] = KEY_1,
    [0x03] = KEY_2,
    [0x04] = KEY_3,
    [0x05] = KEY_4,
    [0x06] = KEY_5,
    [0x07] = KEY_6,
    [0x08] = KEY_7,
    [0x09] = KEY_8,
    [0x0a] = KEY_9,
    [0x0b] = KEY_0,

    [0x0e] = KEY_BACKSPACE,

    [0x10] = KEY_Q,
    [0x11] = KEY_W,
    [0x12] = KEY_E,
    [0x13] = KEY_R,
    [0x14] = KEY_T,
    [0x15] = KEY_Y,
    [0x16] = KEY_U,
    [0x17] = KEY_I,
    [0x18] = KEY_O,
    [0x19] = KEY_P,

    [0x1c] = KEY_ENTER,

    [0x1e] = KEY_A,
    [0x1f] = KEY_S,
    [0x20] = KEY_D,
    [0x21] = KEY_F,
    [0x22] = KEY_G,
    [0x23] = KEY_H,
    [0x24] = KEY_J,
    [0x25] = KEY_K,
    [0x26] = KEY_L,

    [0x2a] = KEY_LSHIFT,

    [0x2c] = KEY_Z,
    [0x2d] = KEY_X,
    [0x2e] = KEY_C,
    [0x2f] = KEY_V,
    [0x30] = KEY_B,
    [0x31] = KEY_N,
    [0x32] = KEY_M,

    [0x39] = KEY_SPACE,
};