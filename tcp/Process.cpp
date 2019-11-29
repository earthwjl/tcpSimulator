#include "Process.h"

bool Process::bindPort(PortID port)
{
	if (_device->bindProcess(port, this))
	{
		_bindPort = port;
		return true;
	}
	else
		return false;
}
