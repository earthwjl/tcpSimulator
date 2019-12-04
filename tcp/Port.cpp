#include "Port.h"
#include "device.h"

Port::Port(Device* device,  short id) :
	_theDevice(device), _theID(id), _writeBuffer(this),_readBuffer(this)
{

}



void Port::receiveSegment(const segment & seg)
{
	if (isAckSegment(seg))
	{
		size_t ackID = seg.ackid;
		_writeBuffer.receiveAck(ackID);
	}
	else
	{
		_readBuffer.readSegment(seg);
	}
}
void Port::sendSegment(const segment& seg)
{
	_theDevice->sendSegment(seg);
}
void Port::writeBuffer(const char* buf, size_t len)
{
	_writeBuffer.write(buf, len);
}