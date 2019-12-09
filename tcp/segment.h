#pragma once

class segment
{
	friend bool isValidSegment(const segment& seg);
public:
	segment();
	~segment();
	unsigned short srcPort;//源端口
	unsigned short dstPort;//目标端口
	size_t id;//序号
	size_t ackid;//确认号
	//struct {
	//	bool digits[4];
	//} offset;

	//这里将offset设为一个字节
	char offset;//偏移
	//struct {
	//	bool digits[6];
	//}	reserved;
	char reserved;//保留
	bool urg;//是否紧急
	bool ack;//是否为确认报文
	bool psh;//是否尽快交付
	bool rst;//是否重置
	bool syn;//是否同步
	bool fin;//是否结束
	unsigned short wndSize;//窗口大小
	unsigned short checkSum;//校验和
	unsigned short urgPtr;//紧急指针
	/* char buffer[32];//数据*/
	void setBuffer(char* buf, size_t len);
	void getBuffer(char* & buf, size_t & len)const;
	size_t bufferLength()const;
	void updateCheckSum();
private:
	char* _buffer;
	size_t _length;
};
bool isValidSegment(const segment& seg);