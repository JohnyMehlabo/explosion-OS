#include "interfaces.h"
#include "memory.h"

static const IPv4Address localIP = 0xe901a8c0;

uint32_t Interfaces_GetLocalIP() {
    return localIP;
}