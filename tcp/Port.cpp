#include "Port.h"
#include "device.h"

Port::Port(Device* device,  short id) :
	_theDevice(device), _theID(id), _writeBuffer(this),_readBuffer(this),_connectPort(0)
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
void Port::setTargetPort(short port)
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
	seg.offset = seg.buffer - (char*)(&seg);
	calcCheckSum(seg);
	_theDevice->sendSegment(seg);
}
void Port::writeBuffer(const char* buf, size_t len)
{
	_writeBuffer.write(buf, len);
}