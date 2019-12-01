#pragma once
#include "device.h"
typedef unsigned short PortID;
#include "device.h"

typedef unsigned short PortID;

class Port
{
public:
	Port(Device* device, PortID id);
	void reset();
	void acceptSegment(const segment& seg);
};