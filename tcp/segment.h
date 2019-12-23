#pragma once
#include <cstddef>
class segment
{
	friend bool isValidSegment(const segment& seg);
public:
	segment();
	segment(const segment& seg);
	~segment();
	unsigned short srcPort;
	unsigned short dstPort;
	size_t id;
	size_t ackid;

	char offset;

	char reserved;
	bool urg;
	bool ack;
	bool psh;
	bool rst;
	bool syn;
	bool fin;
	unsigned short wndSize;
	unsigned short checkSum;
	unsigned short urgPtr;
	void setBuffer(char* buf, size_t len);
	void getBuffer(char* & buf, size_t & len)const;
	size_t bufferLength()const;
	void updateCheckSum();
private:
	char* _buffer;
	size_t _length;
};
bool isValidSegment(const segment& seg);