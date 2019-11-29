#pragma once
#include <iostream>
#include "device.h"
class Process
{
public:
	Process(Device* device,std::istream& in,std::ostream& out);
	bool bindPort(PortID port);
	bool connect(Device* device, PortID port);
	void run();
	~Process();
private:
	Device* _device;
	PortID _bindPort;
};

