#pragma once
#include "net/common.h"
#include "stdbool.h"

void RTL8139_Init();
bool RTL8139_SendPacket(NetPacket packet);