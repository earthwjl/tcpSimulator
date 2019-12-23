#include "Buffer.h"
#include <cstring>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include "Port.h"
#include <exception>
#include <unistd.h>
#include "Process.h"

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

WriteBaseBuffer::WriteBaseBuffer(size_t len) :
	_wndLeft(0), _wndRight(0), _cacheRight(0), _length(0)
{
	_length = len;
	_buffer = new char[len];
	memset(_buffer,0,len);
}

WriteBaseBuffer::~WriteBaseBuffer()
{
	if (_buffer)
		delete[] _buffer;
}

inline size_t WriteBaseBuffer::length()const
{
	return _length;
}

inline char* WriteBaseBuffer::getWindowLeft()const
{
	return _buffer + (_wndLeft % _length);
}
size_t WriteBaseBuffer::getWndLeftId() const
{
	return _wndLeft;
}
inline char* WriteBaseBuffer::getWindowRight()const
{
	return _buffer + (_wndRight % _length);
}
size_t WriteBaseBuffer::getWindowRightId() const
{
	return _wndRight;
}
inline char * WriteBaseBuffer::getBuffer() const
{
	return _buffer;
}
inline char* WriteBaseBuffer::getCacheEnd()const
{
	return _buffer + (_cacheRight % _length);
}
size_t WriteBaseBuffer::getSpareSize()const
{
	return _length - (_cacheRight - _wndLeft);
}
void WriteBaseBuffer::deleteBuffer(size_t & len)
{
	_wndRight = min(_wndRight + len, _cacheRight);
	_wndLeft = min(_wndLeft + len, _wndRight);
}
void WriteBaseBuffer::writeBuffer(const char * buf, size_t len)
{
	if (!_buffer || len == 0)
		return;
	while (getSpareSize() == 0)
	{}

	size_t cacheRight = _cacheRight % _length;
	size_t wndLeft = _wndLeft % _length;
	size_t wndRight = _wndRight % _length;
	if (cacheRight >= wndLeft)
	{
		size_t cpLen = min(len, _length - cacheRight);
		memcpy(_buffer + cacheRight, buf, cpLen);
		len -= cpLen;
		buf += cpLen;
		_cacheRight += cpLen;
	}
	else
	{
		size_t cpLen = min(len, wndLeft - cacheRight);
		memcpy(_buffer + cacheRight, buf, cpLen);
		buf += cpLen;
		len -= cpLen;
		_cacheRight += cpLen;
	}
	if (_wndRight == _wndLeft && _wndRight < _cacheRight)
	{
		_wndRight = _wndLeft + (_cacheRight - _wndLeft) / 2;
	}
	if (len != 0)
		writeBuffer(buf, len);
}
inline size_t WriteBaseBuffer::getCurrentWindowSize()const
{
	if (!_buffer)
		return 0;
	return _wndRight - _wndLeft;
}
char* WriteBaseBuffer::readWindow(size_t& len)
{
	size_t currentWndLen = getCurrentWindowSize();
	if (currentWndLen == 0)
		return NULL;

	if (len > currentWndLen)
		len = currentWndLen;
	char* ret = new char[len];
	size_t wndRight = _wndRight % _length;
	size_t wndLeft = _wndLeft % _length;
	if (wndLeft < wndRight)
	{
		size_t right = min(len, wndRight - wndLeft);
		memcpy(ret, _buffer + wndLeft, right);
	}
	else
	{
		if (len >= _length - _wndLeft)
		{
			size_t right = _length - _wndLeft;
			memcpy(ret, _buffer + wndLeft, right);
			if(len != right)
				memcpy(ret + right, _buffer, min(wndRight, len - right));
		}
		else
		{
			memcpy(ret, _buffer + wndLeft, len);
		}
	}
	return ret;
}

WriteBuffer::WriteBuffer(Port* port, size_t bufferLen) :
	WriteBaseBuffer(bufferLen), _writeThread(std::thread(&WriteBuffer::_writeHandler, this)),
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
	std::cout << "start write to writebuffer" << std::endl;
	_writeMutex.lock();
	_tmpBuffer = buf;
	_tmpLength = len;
	_writeMutex.unlock();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	std::thread sendThread(&WriteBuffer::_sendHandler, this);
	sendThread.join();
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
			size_t wnsSize = getCurrentWindowSize();
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
	if (id > _wndLeft)
	{
		size_t len = id - _wndLeft;
		deleteBuffer(len);
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
			size_t maxLen = 55;
			char* buf = readWindow(maxLen);
			segment theSeg;
			memset(&theSeg, 0, sizeof(segment));
			theSeg.setBuffer(buf, maxLen);
			theSeg.id = getWndLeftId();
			theSeg.ack = false;
			if (_cacheRight == _wndRight)
				theSeg.fin = true;
			delete[] buf;
			sendSegment(theSeg);
			sleep(1);
		}
		else
			break;
	}
}
ReadBaseBuffer::ReadBaseBuffer(size_t len)
	:_wndLeft(0), _wndRight(0), _cacheLeft(0), _length(0)
{
	_buffer = new char[len];
	_length = len;
	memset(_buffer, 0, len);
}
ReadBaseBuffer::~ReadBaseBuffer()
{
	if (_buffer)
		delete[] _buffer;
}
void ReadBaseBuffer::writeBuffer(char* buf, size_t len, size_t pos)
{
	if (pos >= _cacheLeft && pos + len <= _wndLeft)
	{
		size_t dist = _wndLeft - pos;
		buf += dist;
		if (len <= dist)
		{
			std::cout << "received before reject!" << std::endl;
			return;
		}
		len -= dist;
		pos = _wndLeft;
	}
	while (getSpareSize() == 0)
	{
		sleep(100);
	}
	size_t cacheLeft = _cacheLeft % _length;
	size_t wndLeft = _wndLeft % _length;
	size_t wndRight = _wndRight % _length;
	size_t posOnBuffer = pos % _length;

	if (posOnBuffer >= cacheLeft)
	{
		size_t cpLen = min(_length - posOnBuffer, len);
		memcpy(_buffer + posOnBuffer, buf, cpLen);
		len -= cpLen;
		buf += cpLen;

		if (pos + cpLen > _wndRight)
			_wndRight = pos + cpLen;
		if (pos == _wndLeft)
			_wndLeft += cpLen;
		if (len > 0)
		{
			writeBuffer(buf, len, pos + cpLen);
		}
	}
	else
	{
		size_t cpLen = min(len, cacheLeft - posOnBuffer);
		memcpy(_buffer + posOnBuffer, buf, cpLen);
		if (pos + cpLen > _wndRight)
			_wndRight = pos + cpLen;
		len -= cpLen;
		buf += cpLen;
		if (len > 0)
		{
			writeBuffer(buf, len, pos + cpLen);
		}
	}
}
void ReadBaseBuffer::readBuffer(char *& buf, size_t & len)
{
	size_t cacheSize = _wndLeft - _cacheLeft;
	if (len > cacheSize)
		len = cacheSize;
	if (len == 0)
	{
		buf = NULL;
		return;
	}

	char* pRet = new char[len];
	size_t cacheLeft = _cacheLeft % _length;
	if (cacheLeft + len > _length)
	{
		size_t left = cacheLeft + len - _length;
		memcpy(pRet, _buffer + cacheLeft, left);
		memcpy(pRet, _buffer, len - left);
	}
	else
		memcpy(pRet, _buffer + cacheLeft, len);
	
	buf = pRet;
	deleteLength(len);
}
void ReadBaseBuffer::deleteLength(size_t & len)
{
	size_t oldCacheLeft = _cacheLeft;
	_cacheLeft = min(_cacheLeft + len, _wndLeft);
	len = _cacheLeft - oldCacheLeft;
}
size_t ReadBaseBuffer::getSpareSize()const
{
	return _length - (_wndLeft - _cacheLeft);
}
size_t ReadBaseBuffer::getCacheSize()const
{
	return _wndLeft - _cacheLeft;
}

bool ReadBaseBuffer::isFull() const
{
	return (_wndRight - _cacheLeft) == _length;
}

bool ReadBaseBuffer::isEmpty() const
{
	return _cacheLeft == _wndRight;
}

ReadBuffer::ReadBuffer(Port * port, size_t bufLen):ReadBaseBuffer(bufLen), _port(port),_fin(false)
{
}

ReadBuffer::~ReadBuffer()
{
}
bool ReadBuffer::readSegment(const segment & seg)
{
	if (isFull())
	{
		std::cout << "read buffer is full!\n";
		checkUpload();
		return false;
	}
	std::cout << "read segment id = " << seg.id << std::endl;
	char* buf = NULL;
	size_t bufLen = 0;
	seg.getBuffer(buf, bufLen);
	if (bufLen == 0)
		return true;
	if (seg.fin)
		_fin = true;
	if (bufLen)
	{
		writeBuffer(buf, bufLen, seg.id);
		checkUpload();
	}
	sendAck(seg.id + seg.bufferLength());
	return true;
}

void ReadBuffer::sendAck(size_t ackId)
{
	std::cout << "readbuffer send ack " << ackId << std::endl;
	segment ackSegment;
	ackSegment.ack = true;
	ackSegment.ackid = ackId;
	_port->sendSegment(ackSegment);
}



void ReadBuffer::read(char* & buffer, size_t & len)
{

}

void ReadBuffer::checkUpload()
{
	if (!_fin && getSpareSize() > 0)
		return;
	forceUpload();
}

void ReadBuffer::forceUpload()
{
	char* buf = NULL;
	size_t len = _length;
	readBuffer(buf, len);
	if (len > 0)
	{
		_port->uploadBuffer(buf, len);
		delete[] buf;
	}
}
