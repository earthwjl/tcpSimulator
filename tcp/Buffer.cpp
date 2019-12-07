#include "Buffer.h"
#include <cstring>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include "Port.h"
#include <exception>

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

Buffer::Buffer(size_t len):
	_wndLeft(0),_wndRight(0),_cacheRight(0),_length(0)
{
	try {
		if (len < 5 || len == (size_t)(-1))
			throw std::exception("bad buffer length!");
		_length = len;
		_buffer = new char[len];
		memset(_buffer, 0, len);
	}
	catch (std::exception& e)
	{
		_buffer = NULL;
		std::cerr << e.what() << std::endl;
	}
}

Buffer::~Buffer()
{
	if (_buffer)
		delete[] _buffer;
}

inline size_t Buffer::length()const
{
	return _length;
}

inline char* Buffer::getWindowLeft()const
{
	return _buffer + _wndLeft;
}
inline char* Buffer::getWindowRight()const
{
	return _buffer + _wndRight;
}
inline char * Buffer::getBuffer() const
{
	return _buffer;
}
inline char* Buffer::getCacheEnd()const
{
	return _buffer + _cacheRight;
}
size_t Buffer::getSpareSize()const
{
	if (_wndLeft == _cacheRight)
	{
		if (_wndRight != _wndLeft)
			return 0;
		else
			return _length;
	}
	else if (_wndLeft < _cacheRight)
		return _length - (_cacheRight - _wndLeft);
	else
		return _wndLeft - _cacheRight;
}
void Buffer::deleteBuffer(size_t & len)
{
	if (_wndRight <= _cacheRight)
		_wndRight = min(_wndRight + len, _cacheRight);
	else
	{
		if (_wndRight + len >= _length)
			_wndRight = len - (_length - _wndRight);
		else
			_wndRight += len;
	}
	size_t oldLeft = _wndLeft;

	if (_wndLeft <= _wndRight)
	{
		_wndLeft = min(_wndLeft + len, _wndRight);
		len = _wndLeft - oldLeft;
	}
	else
	{
		if (_wndLeft + len >= _length)
		{
			_wndLeft = min(len - (_length - _wndLeft), _wndRight);
			len = _length - oldLeft + _wndLeft;
		}
		else
		{
			_wndLeft += len;
		}
	}
}
void Buffer::writeBuffer(char * buf, size_t len)
{
	if (!_buffer || len == 0)
		return;
	size_t oldLen = len;
	//初始情况下，将窗口长度设为buffer length的一半和len的最小值
	size_t rightSpareSize = _length - _cacheRight;
	size_t leftSpareSize = _wndLeft;
	if (rightSpareSize > 0)
	{
		size_t copyLen = min(len, rightSpareSize);
		memcpy(getCacheEnd(), buf, copyLen);
		buf += copyLen;
		len -= copyLen;
		_cacheRight += copyLen;
		if (len == 0 && getCurrentWindowSize() == 0)
		{
			_wndRight = _wndLeft + min(len, rightSpareSize / 2);
			return;
		}
	}

	if (leftSpareSize > 0)
	{
		size_t cpLen = min(len, _wndLeft);
		memcpy(_buffer, buf, cpLen);
		buf += cpLen;
		len -= cpLen;
		_cacheRight = cpLen;

		if (len == 0 && getCurrentWindowSize() == 0)
		{
			_wndRight = _wndLeft + min(leftSpareSize / 2, rightSpareSize / 2);
			return;
		}
	}
	if (getCurrentWindowSize() == 0)
	{
		size_t right = _length - _wndLeft;
		if (right > _length / 2)
			_wndRight = _wndLeft + right / 2;
		else
			_wndRight = _length - 1;
	}
	clock_t startWaiting = clock();
	while (getSpareSize() == 0)
	{
		std::cout << "no spare size...have been waiting for " << clock() - startWaiting << " ticks" << std::endl;
	}
	writeBuffer(buf, len);
}
inline size_t Buffer::getCurrentWindowSize()const
{
	if (!_buffer)
		return 0;
	if (_wndRight > _wndLeft)
		return _wndRight - _wndLeft;
	else
		return _length - _wndLeft + _wndRight;
}
char* Buffer::readWindow(size_t& len)
{
	size_t currentWndLen = getCurrentWindowSize();
	if (currentWndLen == 0)
		return NULL;

	if (len > currentWndLen)
		len = currentWndLen;
	char* ret = new char[len];
	memcpy(ret, getWindowLeft(), len);
	deleteBuffer(len);
	return ret;
}

WriteBuffer::WriteBuffer(Port* port) :
	Buffer(88), _writeThread(std::thread(&WriteBuffer::_writeHandler, this)),
	_tmpLength(0), _tmpBuffer(NULL), _port(port), _stopThread(false)
{

}
WriteBuffer::~WriteBuffer()
{
 	_stopThread = true;
	_writeThread.join();

	std::lock_guard<std::mutex> guard(_writeMutex);
	if (_tmpBuffer)
		delete[] _tmpBuffer;
	_tmpBuffer = NULL;
}
void WriteBuffer::write(const char* buf, size_t len)
{
	_writeMutex.lock();
	_tmpBuffer = new char[len];
	memcpy(_tmpBuffer, buf, len);
	_tmpLength = len;
	_writeMutex.unlock();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	std::thread sendThread(&WriteBuffer::_sendHandler, this);
	sendThread.detach();
}
void WriteBuffer::_writeHandler()
{
	while (1)
	{
		if (_stopThread)
			break;
		if (_tmpBuffer)
		{
			std::lock_guard<std::mutex> guard(_writeMutex);
			writeBuffer(_tmpBuffer, _tmpLength);
			delete[] _tmpBuffer;
			_tmpBuffer = NULL;
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
}

void WriteBuffer::receiveAck(size_t id)
{
	char* _bufferStart = getBuffer();
	char* _wndLeft = getWindowLeft();
	size_t dist = _wndLeft - _bufferStart;
	size_t len = 0;
	if (id > dist)
	{
		len = id - dist;
		deleteBuffer(len);
	}
	else
	{
		len = length() - dist;
		deleteBuffer(len);
		deleteBuffer(id);
	}
}
void WriteBuffer::sendSegment(segment & seg)
{
	_port->sendSegment(seg);
}
void WriteBuffer::_sendHandler()
{
	while (1)
	{
		if (_stopThread)
			break;
		size_t wndSize = getCurrentWindowSize();
		if (wndSize > 0)
		{
			//将数据填入buffer
			size_t maxLen = 32;
			char* buf = readWindow(maxLen);
			segment theSeg;
			memset(&theSeg, 0, sizeof(segment));
			memcpy(theSeg.buffer, buf, maxLen);
			delete[] buf;
			sendSegment(theSeg);
		}
		else
			break;
	}
}

ReadBuffer::ReadBuffer(Port* port) :Buffer(4096),_port(port)
{

}

bool ReadBuffer::read(char *& buf, size_t & len)
{
	return false;
}

void ReadBuffer::readSegment(const segment & seg)
{
}
