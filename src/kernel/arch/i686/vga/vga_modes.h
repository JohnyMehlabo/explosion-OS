#pragma once
#include "stdint.h"
#include "vga.h"

typedef struct {
    union {
        struct {
            uint8_t palette[16];
            uint8_t modeControl;
            uint8_t overscanRegister;
            uint8_t colorPlaneEnable;
            uint8_t horizontalPanning;
            uint8_t colorSelect;
        };
        uint8_t attributeController[21];
    };

    uint8_t miscellaneousOutputRegister;
    
    union {
        struct {
            uint8_t reset;
            uint8_t clockModeRegister;
            uint8_t mapMask;
            uint8_t characterSelect;
            uint8_t memoryModeRegister;
        }; 
        uint8_t sequencer[5];
    };

    union {
        struct 
        {
            uint8_t setReset;
            uint8_t enableSetReset;
            uint8_t colorCompare;
            uint8_t dataRotate;
            uint8_t readMapSelect;
            uint8_t modeRegister;
            uint8_t miscellaneousRegisterGraphics;
            uint8_t colorDontCare;
            uint8_t bitMask;
        };
        uint8_t graphicsController[9];
    };

    union {
        struct {
            uint8_t horizontalTotal;
            uint8_t horizontalDisplayEnableEnd;
            uint8_t horizontalBlankStart;
            uint8_t horizontalBlankEnd;
            uint8_t horizontalRetraceStart;
            uint8_t horizontalRetraceEnd;
            uint8_t verticalTotal;
            uint8_t overfiowRegister;
            uint8_t presetRowScan;
            uint8_t maximumScanLine;
            uint8_t cursorStart;
            uint8_t cursorEnd;
            uint8_t startAddressHigh;
            uint8_t startAddressLow;
            uint8_t cursorLocationHigh;
            uint8_t cursorLocationLow;
            uint8_t verticalRetraceStart;
            uint8_t verticalRetraceEnd;
            uint8_t verticalDisplayEnableEnd;
            uint8_t logicalWidth;
            uint8_t underlineLocation;
            uint8_t verticalBlankStart;
            uint8_t verticalBlankEnd;
            uint8_t crtcModeControl;
            uint8_t lineCompare;
        };
        uint8_t crtController[25];
    };
} VGARegisters;

const VGARegisters* i686_VGA_GetRegisters(VGAMode mode);