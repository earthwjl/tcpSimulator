#include "Buffer.h"



Buffer::Buffer(size_t len):
	_buffer(new char[len]),_length(len),_spareSize(len)
{
}

Buffer::~Buffer()
{
	delete[] _buffer;
}
