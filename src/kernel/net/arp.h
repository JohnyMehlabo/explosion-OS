#pragma once
#include "stdint.h"
#include "common.h"

void ARP_HandlePacket(NetPacket packet);
void ARP_SendRequest(IPv4Address destAddress);