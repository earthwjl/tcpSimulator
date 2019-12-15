#pragma once
#include "Buffer.h"

class Device;
class Port
{
public:
	Port(Device* device,  unsigned short id);
	void writeBuffer(const char* buf, size_t len);
	void readBuffer(char* & buf, size_t & len);
	void receiveSegment(const segment& seg);
	void sendSegment(segment& seg);
	void setTargetPort(unsigned short port);
	unsigned short ID()const { return _theID; }
private:
	WriteBuffer _writeBuffer;
	ReadBuffer _readBuffer;
	Device* _theDevice;
	unsigned short _theID;
	unsigned short _connectPort;
};