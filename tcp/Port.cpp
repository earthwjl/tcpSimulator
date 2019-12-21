#include "Port.h"
#include "device.h"
#include <iostream>
#include "Process.h"
Port::Port(Device* device, unsigned short id) :
	_theDevice(device), _theID(id), _writeBuffer(this),_readBuffer(this),_connectPort(0)
{

}



void Port::receiveSegment(const segment & seg)
{
	std::cout << "port " << _theID << " receive a segment" << std::endl;
	if (seg.ack)
	{
		std::cout << "port " << _theID << " receive an ack segment with id " << seg.ackid << std::endl;
		_writeBuffer.receiveAck(seg.ackid);
		return;
	}
	_readBuffer.readSegment(seg);
	if (seg.fin)
	{
		Process* bindingProcess = _theDevice->getBindedProcess(_theID);
		if (bindingProcess)
			bindingProcess->unlock();
	}
}
void Port::setTargetPort(unsigned short port)
{
	_connectPort = port;
}
bool firstSegment = false;
void Port::uploadBuffer(char * buf, size_t len)
{
	Process* bindingProcess = _theDevice->getBindedProcess(_theID);
	if (firstSegment == false)
	{
		bindingProcess->lock();
		firstSegment = true;
	}
	if (!bindingProcess)
		return;
	bindingProcess->acceptBuffer(buf, len);
}
void Port::sendSegment(segment& seg)
{
	if (_connectPort == 0)
		return;
	//将数据打包成一个segment;
	seg.srcPort = _theID;
	seg.dstPort = _connectPort;
	seg.updateCheckSum();
	std::cout << "port " << _theID << " send segment" << std::endl;
	_theDevice->sendSegment(seg);
}
void Port::writeBuffer(const char* buf, size_t len)
{
	std::cout << "process write buffer size =" << len << std::endl;
	_writeBuffer.write(buf, len);
}
void Port::readBuffer(char* & buf, size_t & len)
{
	_readBuffer.read(buf, len);
}