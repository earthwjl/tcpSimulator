#include "Buffer.h"
#include <cstring>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include "Port.h"
#include <exception>
#include <Windows.h>

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

WriteBaseBuffer::WriteBaseBuffer(size_t len) :
	_wndLeft(0), _wndRight(0), _cacheRight(0), _length(0)
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
	return _buffer + _wndLeft;
}
size_t WriteBaseBuffer::getWndLeftId() const
{
	return _wndLeft;
}
inline char* WriteBaseBuffer::getWindowRight()const
{
	return _buffer + _wndRight;
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
	return _buffer + _cacheRight;
}
size_t WriteBaseBuffer::getSpareSize()const
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
void WriteBaseBuffer::deleteBuffer(size_t & len)
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
void WriteBaseBuffer::writeBuffer(char * buf, size_t len)
{
	if (!_buffer || len == 0)
		return;
	size_t oldLen = len;
	//初始情况下，将窗口长度设为buffer length的一半和len的最小值
	size_t rightSpareSize = _length - _cacheRight;
	size_t leftSpareSize = _wndLeft;
	if (rightSpareSize > 0)
	{
		size_t copyLen = min(oldLen, rightSpareSize);
		memcpy(getCacheEnd(), buf, copyLen);
		buf += copyLen;
		len -= copyLen;
		_cacheRight += copyLen;
		if (len == 0 && getCurrentWindowSize() == 0)
		{
			_wndRight = _wndLeft + min(copyLen, rightSpareSize / 2);
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
inline size_t WriteBaseBuffer::getCurrentWindowSize()const
{
	if (!_buffer)
		return 0;
	if (_wndRight >= _wndLeft)
		return _wndRight - _wndLeft;
	else
		return _length - _wndLeft + _wndRight;
}
char* WriteBaseBuffer::readWindow(size_t& len)
{
	size_t currentWndLen = getCurrentWindowSize();
	if (currentWndLen == 0)
		return NULL;

	if (len > currentWndLen)
		len = currentWndLen;
	char* ret = new char[len];
	memcpy(ret, getWindowLeft(), len);
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
	_writeMutex.lock();
	_tmpBuffer = new char[len];
	memcpy(_tmpBuffer, buf, len);
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
	size_t wndLeft = getWndLeftId();
	if (id >= wndLeft)
	{
		size_t len = id - wndLeft;
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
			//将数据填入buffer
			size_t maxLen = 32;
			char* buf = readWindow(maxLen);
			segment theSeg;
			memset(&theSeg, 0, sizeof(segment));
			theSeg.setBuffer(buf, maxLen);
			theSeg.id = getWndLeftId();
			theSeg.ack = false;
			if (getCacheEnd() == getWindowRight())
				theSeg.fin = true;
			delete[] buf;
			sendSegment(theSeg);
			Sleep(100);
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
	//起始位置在已读取区,则该部分不再处理
	if (pos >= _cacheLeft && pos < _wndLeft)
	{
		size_t dist = _wndLeft - pos;
		buf += dist;
		if (len <= dist)
			return;
		len -= dist;
		pos = _wndLeft;
	}

	bool updateWindowLeft = (pos == _wndLeft);
	size_t bufEnds = pos;
	size_t copyLen = 0;
	if (_wndLeft >= _cacheLeft)
	{
		copyLen = min(len, _length - _wndLeft);
		memcpy(_buffer + _wndLeft, buf, copyLen);
		buf += copyLen;
		len -= copyLen;
		bufEnds += copyLen;
		if (len > 0)
		{
			copyLen = min(_cacheLeft, len);
			memcpy(_buffer, buf, copyLen);
			bufEnds = copyLen;
			buf += copyLen;
			len -= copyLen;
		}
		if (bufEnds <= _wndLeft || bufEnds > _wndRight)
			_wndRight = bufEnds;
		if (updateWindowLeft)
			_wndLeft = bufEnds;

		if (len > 0)
		{
			while (getSpareSize() == 0)
			{
				std::cout << "waiting process to pick up" << std::endl;
			}
			writeBuffer(buf, len, bufEnds);
		}
	}
	else
	{
		copyLen = min(_cacheLeft - _wndLeft, len);
		memcpy(_buffer + _wndLeft, buf, copyLen);
		buf += copyLen;
		len -= copyLen;
		bufEnds = _wndLeft + copyLen;
		if (bufEnds > _wndRight)
			_wndRight = bufEnds;
		if (updateWindowLeft)
			_wndLeft = bufEnds;
		if (len > 0)
		{
			while (getSpareSize() == 0)
			{
				std::cout << "waiting process to pick up" << std::endl;
			}
			writeBuffer(buf, len, bufEnds);
		}
	}
}
void ReadBaseBuffer::readBuffer(char *& buf, size_t & len)
{
	size_t cacheSize = getCacheSize();
	if (cacheSize < len)
		len = cacheSize;
	char* ret = new char[len];
	size_t oldLen = len;
	size_t copyLen = min(_length - _cacheLeft, len);
	memcpy(ret, _buffer + _cacheLeft, copyLen);
	oldLen -= copyLen;
	if (oldLen > 0)
		memcpy(ret + copyLen, _buffer, oldLen);
	buf = ret;
}
void ReadBaseBuffer::deleteLength(size_t & len)
{
	size_t cacheSz = getCacheSize();
	size_t oldCacheLeft = _cacheLeft;

	if (_cacheLeft <= _wndLeft)
	{
		_cacheLeft = min(_cacheLeft + cacheSz, min(_cacheLeft + len, _wndLeft));
		len = _cacheLeft - oldCacheLeft;
	}
	else
	{
		if (_cacheLeft + len > _length)
		{
			_cacheLeft = min((len - (_length - _cacheLeft)), _wndLeft);
			len = _length - oldCacheLeft + _cacheLeft;
		}
		else
			_cacheLeft += len;
	}
}
size_t ReadBaseBuffer::getSpareSize()const
{
	if (_wndRight >= _cacheLeft)
		return _length - (_wndRight - _cacheLeft);
	else
		return _cacheLeft - _wndRight;
}
size_t ReadBaseBuffer::getCacheSize()const
{
	if (_cacheLeft <= _wndLeft)
		return _wndLeft - _cacheLeft;
	else
		return _wndLeft + (_length - _cacheLeft);
}

ReadBuffer::ReadBuffer(Port * port, size_t bufLen):ReadBaseBuffer(bufLen), _port(port),_fin(false)
{
}

ReadBuffer::~ReadBuffer()
{
}

void ReadBuffer::readSegment(const segment & seg)
{
	char* buf = NULL;
	size_t bufLen = 0;
	seg.getBuffer(buf, bufLen);
	if (bufLen == 0)
		return;
	if (seg.fin)
		_fin = true;
	if (bufLen)
	{
		writeBuffer(buf, bufLen, seg.id);
	}
	sendAck(_wndLeft);
}

void ReadBuffer::sendAck(size_t ackId)
{
	segment ackSegment;
	ackSegment.ack = true;
	ackSegment.ackid = ackId;
	_port->sendSegment(ackSegment);
}

size_t ReadBuffer::getBufferSize() const
{
	if (_wndLeft >= _cacheLeft)
		return _wndLeft - _cacheLeft;
	else
		return (_length - _cacheLeft) + _wndLeft;
}


void ReadBuffer::read(char* & buffer, size_t & len)
{
	clock_t startTime = clock();
	char* localBuffer = NULL;
	size_t localLen = 0;
	while (1)
	{
		if (getBufferSize() > 0)
		{
			clock_t duration = clock() - startTime;
			if (getBufferSize() > _length / 4 || duration > 200)
			{
				size_t currentLength = getBufferSize();
				if (localLen == 0)
				{
					localLen = currentLength;
					localBuffer = new char[currentLength];
					memcpy(localBuffer, _buffer + _cacheLeft, currentLength);
				}
				else
				{
					char* dst = new char[localLen + currentLength];
					memcpy(dst, localBuffer, localLen);
					memcpy(dst + localLen, _buffer + _cacheLeft, currentLength);
					delete[] localBuffer;
					localBuffer = dst;
					localLen += currentLength;
				}
				buffer = localBuffer;
				len = localLen;
				startTime = clock();
				deleteLength(currentLength);
			}
		}
		if (_fin)
			break;
	}
}
