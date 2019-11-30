#pragma once
#include <thread>
#include <mutex>

class Buffer
{
public:
	size_t length()const;
protected:
	Buffer(size_t length);
	virtual ~Buffer();
	char* readWindow(size_t len);
	char* getWindowLeft()const;
	char* getWindowRight()const;
	char* getCacheEnd()const;
	void writeBuffer(char* buf,size_t len);
	size_t getCurrentWindowSize()const;
	size_t getSpareSize()const;
	void deleteBuffer(size_t len);
private:
	char* _buffer;
	size_t _wndLeft;
	size_t _wndRight;
	size_t _cacheRight;
	size_t _length;
};

class WriteBuffer :public Buffer
{
public:
	WriteBuffer();
	~WriteBuffer();
	void write(char* buf, size_t len);
private:
	static void _writeHandler(WriteBuffer* buffer);
	static void _sendHandler(WriteBuffer* buffer);
	std::thread _sendThread;
	std::thread _writeThread;
	std::mutex _writeMutex;
	char* _tmpBuffer;
	size_t _tmpLength;
};
