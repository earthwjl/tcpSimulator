#pragma once
#include <iostream>
#include "device.h"
class Process
{
public:
	Process(Device* device,std::istream& in,std::ostream& out);
	bool bindPort(unsigned short port);
	 short getBindingPort()const;
	bool connect(Device* device, unsigned short port);
	void run();
	~Process();
private:

private:
	Device* _device;
	unsigned short _bindPort;
	std::istream& _instream;
	std::ostream& _outstream;
	unsigned short _targetPort;
};

