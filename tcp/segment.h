#pragma once
struct segment
{
	 short srcPort;
	 short dstPort;
	unsigned int id;
	size_t ackid;
	//struct {
	//	bool digits[4];
	//} offset;

	//这里将offset设为一个字节
	char offset;
	//struct {
	//	bool digits[6];
	//}	reserved;
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
	char buffer[32];
};
void calcCheckSum(segment& seg);
bool isValidSegment(const segment& seg);
bool isAckSegment(const segment& seg);
bool getAckSegmentNumber(const segment& seg,size_t & number);
