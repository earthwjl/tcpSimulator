#pragma once

class segment
{
	friend bool isValidSegment(const segment& seg);
public:
	segment();
	~segment();
	unsigned short srcPort;//Դ�˿�
	unsigned short dstPort;//Ŀ��˿�
	size_t id;//���
	size_t ackid;//ȷ�Ϻ�
	//struct {
	//	bool digits[4];
	//} offset;

	//���ｫoffset��Ϊһ���ֽ�
	char offset;//ƫ��
	//struct {
	//	bool digits[6];
	//}	reserved;
	char reserved;//����
	bool urg;//�Ƿ����
	bool ack;//�Ƿ�Ϊȷ�ϱ���
	bool psh;//�Ƿ񾡿콻��
	bool rst;//�Ƿ�����
	bool syn;//�Ƿ�ͬ��
	bool fin;//�Ƿ����
	unsigned short wndSize;//���ڴ�С
	unsigned short checkSum;//У���
	unsigned short urgPtr;//����ָ��
	/* char buffer[32];//����*/
	void setBuffer(char* buf, size_t len);
	void getBuffer(char* & buf, size_t & len)const;
	size_t bufferLength()const;
	void updateCheckSum();
private:
	char* _buffer;
	size_t _length;
};
bool isValidSegment(const segment& seg);