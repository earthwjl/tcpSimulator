#include "Process.h"
#include "pipe.h"
#include <string>
#include <fstream>

Process::Process(Device * device, std::istream & in, std::ostream & out) :
	_instream(in), _outstream(out), _device(device),_bindPort(0),_targetPort(0)
{
}

bool Process::bindPort(unsigned short port)
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

bool Process::connect(Device * device,unsigned short port)
{
	if (_bindPort == 0)
		return false;
	Pipe* thePipe = Pipe::getInstance();
	thePipe->bind(_device, device);
	_targetPort = port;

	Port* _port = _device->getPort(_bindPort);
	if (_port)
		_port->setTargetPort(_targetPort);
	Port* _port1 = device->getPort(port);
	if (_port1)
		_port1->setTargetPort(_bindPort);

	return true;
}

void Process::write()
{
	std::string str;
	for (int i = 0; i < 10; ++i)
	{
		str += std::string("1234567890");
	}
	std::cout << "prepare to send buffer size=" << str.size() << std::endl;
	if (_targetPort != 0)
	{
		Port* _port = _device->getPort(_bindPort);
		if (_port)
		{
			_port->writeBuffer(str.c_str(), str.size()+1);
		}
	}
}

Process::~Process()
{
	Pipe* thePipe = Pipe::getInstance();
	thePipe->releaseBind();
}

void Process::read()
{
	char* buffer = NULL;
	size_t bufferLength = 0;

	Port* port = _device->getPort(_bindPort);
	if (port)
	{
		while (1)
		{
			port->readBuffer(buffer, bufferLength);
			if (bufferLength > 0)
			{
				std::vector<char> tmp;
				for (size_t i = 0; i < bufferLength; ++i)
				{
					tmp.push_back(buffer[i]);
				}
				delete[] buffer;
				std::cout << "received size = " << tmp.size() << std::endl;
				std::cout << "received:" << tmp.data() << std::endl;
				break;
			}
		}
	}
}
