#include "Process.h"
#include "pipe.h"
#include <string>

Process::Process(Device * device, std::istream & in, std::ostream & out) :
	_instream(in), _outstream(out), _device(device),_bindPort(0),_targetPort(0)
{
}

bool Process::bindPort(short port)
{
	if (port == 0)
		return false;
	_bindPort = port;
	_device->processBindPort(this, port);
	return true;
}

 short Process::getBindingPort() const
{
	return _bindPort;
}

bool Process::connect(Device * device, short port)
{
	if (_bindPort == 0)
		return false;
	Pipe* thePipe = Pipe::getInstance();
	thePipe->bind(_device, device);
	_targetPort = port;

	Port* _port = _device->getPort(_bindPort);
	if (_port)
		_port->setTargetPort(_targetPort);

	return true;
}

void Process::run()
{
	const char* buf = "1234567812345678123456781234567812345678";
	if (_targetPort != 0)
	{
		Port* _port = _device->getPort(_bindPort);
		if (_port)
		{
			_port->writeBuffer(buf, strlen(buf) + 1);
		}
	}
}

Process::~Process()
{
	Pipe* thePipe = Pipe::getInstance();
	thePipe->releaseBind();
}
