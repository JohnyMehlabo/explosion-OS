#include "mem_detection.h"

uint16_t MemDect_DetectMemory(void* outAddr) {
    return x86_DetectMemory(outAddr);
}