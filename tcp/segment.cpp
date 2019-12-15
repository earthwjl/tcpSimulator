#include "segment.h"
#include <cstring>

bool isValidSegment(const segment & seg)
{
	unsigned int len = sizeof(segment) / sizeof(unsigned short);
	unsigned short result = 0;
	unsigned short* head = (unsigned short*)(&seg);
	for (unsigned int i = 0; i < len; ++i)
	{
		result += head[i];
	}
	return result == (unsigned short)(-1);
}


segment::segment() :
	srcPort(0),
	dstPort(0),
	id(0),
	ackid(0),
	offset(0),
	reserved(0),
	urg(false),
	ack(false),
	psh(false),
	rst(false),
	syn(false),
	fin(false),
	wndSize(0),
	checkSum(0),
	urgPtr(0),
	_buffer(NULL),
	_length(0)
{
	
}

segment::segment(const segment & seg)
{
	if (seg._buffer || seg._length)
	{
		_buffer = new char[seg._length];
		memcpy(_buffer, seg._buffer, seg._length);
		_length = seg._length;
	}
	srcPort = seg.srcPort;
	dstPort = seg.dstPort;
	id = seg.id;
	ackid = seg.ackid;
	offset = seg.offset;
	reserved = seg.reserved;
	urg = seg.urg;
	ack = seg.ack;
	psh = seg.psh;
	rst = seg.rst;
	syn = seg.syn;
	fin = seg.fin;
	wndSize = seg.wndSize;
	urgPtr = seg.urgPtr;
	if (seg.checkSum)
	{
		updateCheckSum();
	}
}

segment::~segment()
{
	if (_buffer)
		delete[] _buffer;
}

void segment::setBuffer(char * buf, size_t len)
{
	_length = len;
	_buffer = new char[len];
	memcpy(_buffer, buf, len);
}

void segment::getBuffer(char *& buf, size_t & len)const
{
	len = _length;
	if (_length == 0)
	{
		buf = NULL;
		return;
	}
	buf = _buffer;
}

size_t segment::bufferLength()const
{
	return _length;
}

void segment::updateCheckSum()
{
	checkSum = 0;
	unsigned short* p = (unsigned short*)this;
	size_t len = sizeof(segment) / sizeof(unsigned short);
	unsigned short result = 0;
	for (size_t i = 0; i < len; ++i)
	{
		result += p[i];
	}
	checkSum = ~result;
}
