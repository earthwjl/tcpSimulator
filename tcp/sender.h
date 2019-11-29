#pragma once
#include "device.h"

class Sender
{
public:
	Sender();
	void send(Device* device, PortID port);
};


