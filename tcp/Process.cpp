#include "Process.h"
#include "pipe.h"
#include <string>
#include <fstream>

Process::Process(Device * device, std::istream & in, std::ostream & out) :
	_instream(in), _outstream(out), _device(device),_bindPort(0),_targetPort(0)
{
	lockStatus = false;
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
	std::this_thread::sleep_for(std::chrono::seconds(3));
	if(_buffer.size())
		std::cout <<"buffer size = " << _buffer.size() << "buffer is " <<   _buffer.data() << std::endl;
}

void Process::acceptBuffer(char * buf, size_t len)
{
	lock();
	for (size_t i = 0; i < len; ++i)
		_buffer.push_back(buf[i]);
	unlock();
}
void Process::lock()
{
	if (lockStatus)
		return;
	_rwMutex.lock();
	lockStatus = true;
}
void Process::unlock()
{
	if (lockStatus == false)
		return;
	_rwMutex.unlock();
	lockStatus = false;
}