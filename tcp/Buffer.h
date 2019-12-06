#pragma once
#include <thread>
#include <mutex>
#include "segment.h"
class Port;

class Buffer
{
public:
	size_t length()const;
	void reset(size_t length);
protected:
	Buffer(size_t length);
	virtual ~Buffer();
	char* readWindow(size_t& len);
	char* getWindowLeft()const;
	char* getWindowRight()const;
	char* getBuffer()const;
	char* getCacheEnd()const;
	void writeBuffer(char* buf,size_t len);
	size_t getCurrentWindowSize()const;
	size_t getSpareSize()const;
	//������buffer��Ϊ�Ѷ�
	void deleteBuffer(size_t & len);
private:
	char* _buffer;
	size_t _wndLeft;
	size_t _wndRight;
	size_t _cacheRight;
	size_t _length;
};

class WriteBuffer :public Buffer
{
	friend class Port;
public:
	WriteBuffer(Port* port);
	std::mutex _stopMutex;

	char* _tmpBuffer;

	~WriteBuffer();
	void write(const char* buf, size_t len);
private:
	void _writeHandler();
	void _sendHandler();
	void receiveAck(size_t id);
	void sendSegment(segment& seg);
	std::mutex _writeMutex;
	size_t _tmpLength;
	Port* _port;
	bool terminateThread;
	std::thread* _sendThread;
	std::thread* _writeThread;
};
class ReadBuffer :public Buffer
{
	friend class Port;
public:
	ReadBuffer(Port* port);
	bool read(char* &  buf, size_t& len);
private:
	void readSegment(const segment& seg);
private:
	Port* _port;

};
