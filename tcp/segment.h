#pragma once
struct segment
{
	unsigned short srcPort;
	unsigned short dstPort;
	unsigned int id;
	unsigned int ackid;
	struct {
		bool digits[4];
	} offset;
	struct {
		bool digits[6];
	}	reserved;
	bool urg;
	bool ack;
	bool psh;
	bool rst;
	bool syn;
	bool fin;
	unsigned short wndSize;
	unsigned short sum;
	unsigned short urgPtr;
};

