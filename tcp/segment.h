#pragma once
struct segment
{
	unsigned short srcPort;
	unsigned short dstPort;
	unsigned int id;
	unsigned int ackid;
	//struct {
	//	bool digits[4];
	//} offset;

	//���ｫoffset��Ϊһ���ֽ�
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
