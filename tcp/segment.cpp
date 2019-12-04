#include "segment.h"

void calcCheckSum(segment & seg)
{
	seg.checkSum = 0;
	unsigned int len = sizeof(seg) / sizeof(short);
	unsigned short* head = (unsigned short*)(&seg);
	unsigned short result = 0;
	for (unsigned int i = 0; i < len; ++i)
	{
		result += head[i];
	}
	seg.checkSum = ~result;
}

bool isValidSegment(const segment & seg)
{
	unsigned int len = sizeof(segment) / sizeof(unsigned short);
	unsigned short result = 0;
	unsigned short* head = (unsigned short*)(&seg);
	for (int i = 0; i < len; ++i)
	{
		result += head[i];
	}
	return result == (unsigned short)(-1);
}
bool isAckSegment(const segment& seg)
{
	if (seg.offset == sizeof(segment))
		return true;
	else
		return false;
}
bool getAckSegmentNumber(const segment& seg, size_t & number)
{
	if (!isAckSegment(seg))
		return false;
	number = seg.ackid;
	return true;
}