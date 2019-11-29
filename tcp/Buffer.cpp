#include "Buffer.h"
#include <iostream>


Buffer::Buffer(size_t len) :
	_buffer(new char[len]), _length(len)
{
	memset(_buffer, 0, len);
	_firstSpareBlock = _buffer;
	_firstOccupiedBlock = _buffer + _length;
}

Buffer::~Buffer()
{
	delete[] _buffer;
}

bool Buffer::isFull() const
{
	return _firstSpareBlock == _buffer + _length;
}
bool Buffer::isEmpty()const
{
	return _firstSpareBlock == _buffer && (_firstOccupiedBlock == (_buffer + _length));
}
