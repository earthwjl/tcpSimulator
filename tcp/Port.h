#pragma once
#include "Buffer.h"

class Device;
class Port
{
public:
	Port(Device* device,  short id);
	void writeBuffer(const char* buf, size_t len);
	void receiveSegment(const segment& seg);
	void sendSegment(segment& seg);
	void setTargetPort(short port);
	unsigned short ID()const { return _theID; }
private:
	WriteBuffer _writeBuffer;
	ReadBuffer _readBuffer;
	Device* _theDevice;
	unsigned short _theID;
	short _connectPort;
};