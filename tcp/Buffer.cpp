#include "Buffer.h"
#include <cstring>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include "Port.h"

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
	if (_cacheRight < _wndLeft)
		return _wndLeft - _cacheRight;
	else
		return _length - (_cacheRight - _wndLeft);
}
void Buffer::deleteBuffer(size_t & len)
{
	size_t oldLeft = _wndLeft;
	if (_cacheRight < _wndLeft)
	{
		if (_wndRight + len >= _length)
		{
			if (_wndRight + len - _length < _cacheRight)
				_wndRight = _wndRight + len - _length;
			else
				_wndRight = _cacheRight;

			if (_wndLeft + len >= _length)
				_wndLeft = min(_wndLeft + len - _length, _wndRight);
			else
				_wndLeft += len;
		}
		else
		{
			if (_wndRight > _wndLeft)
			{
				if (_wndRight + len < _length)
					_wndRight += len;
				else
					_wndRight = min(_cacheRight, _wndRight + len - _length);
				if (_wndLeft + len > _length)
					_wndLeft = min(_wndRight, _wndLeft + len - _length);
				else
					_wndLeft += len;
			}
			else
			{
				_wndRight = min(_cacheRight, _wndRight + len);
				if (_wndLeft + len > _length)
					_wndLeft = min(_wndLeft + len - _length, _wndRight);
				else
					_wndLeft += len;
			}
		}
	}
	else
	{
		if (_wndRight + len >= _cacheRight)
			_wndRight = _cacheRight;
		else
			_wndRight += len;
		if (_wndLeft + len >= _wndRight)
			_wndLeft = _wndRight;
		else
			_wndLeft += len;
	}
	if (oldLeft < _wndLeft)
		len = _wndLeft - oldLeft;
	else
		len = _wndLeft + (_length - _wndLeft);
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
	if (_wndRight > _wndLeft)
		return _wndRight - _wndLeft;
	else
		return _wndRight + (_length - _wndLeft);
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

bool stopThread = false;

WriteBuffer::WriteBuffer(Port* port) :
	Buffer(4096),_sendThread(new std::thread(&WriteBuffer::_sendHandler,this)),_writeThread(new std::thread(&WriteBuffer::_writeHandler,this)),
	_tmpLength(0),_tmpBuffer(NULL),_port(port),terminateThread(false)
{

}
WriteBuffer::~WriteBuffer()
{
	stopThread = true;
	_sendThread->join();
	_writeThread->join();
	delete _writeThread;
	delete _sendThread;
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
}
void WriteBuffer::_writeHandler()
{
	while (1)
	{
		if (stopThread)
			break;
		if (_tmpBuffer)
		{
			std::lock_guard<std::mutex> guard(_writeMutex);
			writeBuffer(_tmpBuffer, _tmpLength);
			delete[] _tmpBuffer;
			_tmpBuffer = NULL;
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
		if (stopThread)
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
