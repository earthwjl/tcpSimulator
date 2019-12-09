#pragma once
#include <thread>
#include <mutex>
#include "segment.h"
class Port;

class WriteBaseBuffer
{
public:
	size_t length()const;
protected:
	WriteBaseBuffer(size_t length);
	virtual ~WriteBaseBuffer();
	char* readWindow(size_t& len);
	char* getWindowLeft()const;
	size_t getWndLeftId()const;
	char* getWindowRight()const;
	size_t getWindowRightId()const;
	char* getBuffer()const;
	char* getCacheEnd()const;
	void writeBuffer(char* buf,size_t len);
	size_t getCurrentWindowSize()const;
	size_t getSpareSize()const;
	//将部分buffer标为已读
	void deleteBuffer(size_t & len);
private:
	char* _buffer;
	size_t _wndLeft;
	size_t _wndRight;
	size_t _cacheRight;
	size_t _length;
};

class WriteBuffer :public WriteBaseBuffer
{
	friend class Port;
public:
	WriteBuffer(Port* port,size_t bufferLen = 4096);
	~WriteBuffer();
	void write(const char* buf, size_t len);
private:
	void _writeHandler();
	void _sendHandler();
	void receiveAck(size_t id);
	void sendSegment(segment& seg);
private:
	char* _tmpBuffer;
	size_t _tmpLength;
	bool _stopThread;
	std::mutex _writeMutex;
	Port* _port;
	std::thread _writeThread;
};
class ReadBaseBuffer
{
public:
	size_t getCacheLeft()const { return _cacheLeft; }
	size_t getWndLeft()const { return _wndLeft; }
	size_t getWndRight()const { return _wndRight; }
	size_t getCacheSize()const;
protected:
	ReadBaseBuffer(size_t len);
	~ReadBaseBuffer();
	void writeBuffer(char* buf, size_t len, size_t pos);
	void readBuffer(char* & buf, size_t & len);
	void deleteLength(size_t& len);
	size_t getSpareSize()const;
	char* _buffer;
	size_t _length;
	size_t _cacheLeft;
	size_t _wndLeft;
	size_t _wndRight;
};
class ReadBuffer :public ReadBaseBuffer
{

};