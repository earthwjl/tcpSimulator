#include "device.h"
#include "Process.h"
#include <iostream>
#include <fstream>
#include "pipe.h"
#include "Port.h"

Device::Device()
{}
Device::~Device()
{
	for (auto iter : _portMap)
	{
		delete iter.second;
	}
	for (Process* p : _processSet)
	{
		delete p;
	}
}
bool Device::processBindPort(Process * process,  short id)
{
	if (!process)
		return false;
	if (id == 0)
		return false;

	if (!_portProcessMap.count(id) || _portProcessMap[id] == NULL)
	{
		_portProcessMap[id] = process;
		//if (_portMap.count(id))
		//	_portMap[id]->reset();
		//else
		_portMap[id] = new Port(this,id);
		return true;
	}
	else
		return false;
}
void Device::processReleasePort(Process* process)
{
	if (!process)return;
	unsigned short bindingPort = process->getBindingPort();
	if (0 == process->getBindingPort())
		return;

	if (_portProcessMap.count(bindingPort))
		_portProcessMap.erase(bindingPort);
}

Process * Device::createProcess()
{
	Process* ret = new Process(this, std::cin, std::cout);
	_processSet.insert(ret);
	return ret;
}
void Device::deleteProcess(Process* process)
{
	delete process;
	if (_processSet.count(process))
		_processSet.erase(process);
}

Port * Device::getPort( short id)
{
	if (_portMap.count(id))
		return _portMap[id];
	else
		return NULL;
}

void Device::sendSegment(const segment& segment)
{
	Pipe* thePipe = Pipe::getInstance();
	Device* a = NULL,*b = NULL;
	thePipe->getDevice(a, b);
	thePipe->sendSegment(this, segment);
}

void Device::getSegment(const segment & seg)
{
	//对报文进行校验
	if (!isValidSegment(seg))
		return;
	unsigned short targetPort = seg.dstPort;
	if (_portMap.count(targetPort))
		_portMap[targetPort]->receiveSegment(seg);
}
