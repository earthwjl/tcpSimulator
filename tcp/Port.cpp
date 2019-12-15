#include "Port.h"
#include "device.h"

Port::Port(Device* device, unsigned short id) :
	_theDevice(device), _theID(id), _writeBuffer(this),_readBuffer(this),_connectPort(0)
{

}



void Port::receiveSegment(const segment & seg)
{
	if (seg.ack)
	{
		_writeBuffer.receiveAck(seg.ackid);
		return;
	}
	_readBuffer.readSegment(seg);
}
void Port::setTargetPort(unsigned short port)
{
	_connectPort = port;
}
void Port::sendSegment(segment& seg)
{
	if (_connectPort == 0)
		return;
	//将数据打包成一个segment;
	seg.srcPort = _theID;
	seg.dstPort = _connectPort;
	seg.updateCheckSum();
	_theDevice->sendSegment(seg);
}
void Port::writeBuffer(const char* buf, size_t len)
{
	_writeBuffer.write(buf, len);
}
void Port::readBuffer(char* & buf, size_t & len)
{
	_readBuffer.read(buf, len);
}