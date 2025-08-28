#pragma once
#include "stdint.h"
#include "common.h"

void Ethernet_SetMACAddress(MACAddress addr);
void Ethernet_GetMACAddress(MACAddress out);

void Ethernet_HandlePacket(NetPacket packet);
NetPacket Ethernet_CreatePacket(MACAddress dest, uint16_t etherType, uint32_t payloadSize);
void Ethernet_SendPacket(NetPacket packet);
