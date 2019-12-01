#include "Buffer.h"
#include <cstring>
#include <iostream>
#include <ctime>
#include <cstdlib>

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

Buffer::Buffer(size_t length):
	_buffer(new char[length]),_length(length),_wndLeft(0),
	_wndRight(0),_cacheRight(0)
{
	if (_buffer)
		memset(_buffer, 0, length);
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
void Buffer::reset(size_t len)
{
	if (_length != len)
	{
		delete[] _buffer;
		_buffer = new char[len];
		_length = len;
	}
	_wndLeft = _wndRight = 0;
	_cacheRight = _wndRight;
}
inline char* Buffer::getWindowLeft()const
{
	return _buffer + _wndLeft;
}
inline char* Buffer::getWindowRight()const
{
	return _buffer + _wndRight;
}
inline char* Buffer::getCacheEnd()const
{
	return _buffer + _cacheRight;
}
size_t Buffer::getSpareSize()const
{
	if (_cacheRight < _wndLeft)
		return _wndLeft - _cacheRight;
	else
		return _length - (_cacheRight - _wndLeft);
}
void Buffer::deleteBuffer(size_t len)
{
	//默认情况下，避免窗口越来越小
	if (_wndRight + len >= _length)
	{
		if (_cacheRight < _wndLeft)
			_wndRight = _length;
		else
		{
			size_t supposedEnd = _wndRight + len;
			_wndRight = min(supposedEnd, _cacheRight);
		}
	}
	_wndLeft = min(_wndRight, _wndLeft + len);
	if (_wndLeft == _length)
		_wndLeft = _wndRight = 0;
}
void Buffer::writeBuffer(char * buf, size_t len)
{
	if (getCurrentWindowSize() > 0)
	{
		size_t rightSpareSize = _length - _cacheRight;
		if (len <= rightSpareSize)
		{
			memcpy(getCacheEnd(), buf, len);
			_cacheRight += len;
		}
		else
		{
			clock_t waitStartTick = clock();
			while (getSpareSize() < len + 5)
			{
				std::cout << "waiting to write buffer...have been waiting for "<< clock() - waitStartTick << "ticks" << std::endl;
			}
			memcpy(getCacheEnd(), buf, rightSpareSize);
			memcpy(_buffer, buf + rightSpareSize, len - rightSpareSize);
			_cacheRight = len - rightSpareSize;
		}
	}
	else
	{
		if (len > 5)
		{
			memcpy(getWindowLeft(), buf, 5);
			_wndRight = _wndLeft + 5;
			_cacheRight = _wndRight;
			writeBuffer(buf + 5, len - 5);
		}
		else
		{
			memcpy(getWindowLeft(), buf, len);
			_wndRight = _wndLeft + len;
			_cacheRight = _wndRight;
		}
	}
}
inline size_t Buffer::getCurrentWindowSize()const
{
	return _wndRight - _wndLeft;
}
char* Buffer::readWindow(size_t len)
{
	size_t currentWndLen = getCurrentWindowSize();
	if (len > currentWndLen)
		len = currentWndLen;
	char* ret = new char[len];
	memcpy(ret, getWindowLeft(), len);
	return ret;
}


WriteBuffer::WriteBuffer() :
	Buffer(4096),_sendThread(std::thread(_sendHandler,this)),_writeThread(std::thread(_writeHandler,this)),
	_tmpLength(0),_tmpBuffer(nullptr)
{
	_writeThread.join();
	_sendThread.join();
}
WriteBuffer::~WriteBuffer()
{
	if (_tmpBuffer)
		delete[] _tmpBuffer;
}
void WriteBuffer::write(char* buf, size_t len)
{
	_writeMutex.lock();
	_tmpBuffer = new char[len];
	memcpy(_tmpBuffer, buf, len);
	_tmpLength = len;
	_writeMutex.unlock();
}
void WriteBuffer::_writeHandler(WriteBuffer* buffer)
{
	while (1)
	{
		if (buffer->_tmpBuffer)
		{
			buffer->_writeMutex.lock();
			buffer->writeBuffer(buffer->_tmpBuffer, buffer->_tmpLength);
			delete[] buffer->_tmpBuffer;
			buffer->_tmpBuffer = NULL;
			buffer->_writeMutex.unlock();
		}
	}
}