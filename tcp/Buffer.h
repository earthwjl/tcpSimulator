#pragma once
class BufferBlock
{
public:
	BufferBlock(char* start, size_t len);
	void setVal(char val);
};
class Buffer
{
public:
	Buffer(size_t len);
	virtual ~Buffer();
	size_t length() const { return _length; }
	bool isFull()const { return _spareSize == 0; }
	bool isEmpty()const { return _spareSize == _length; }

	char* read(size_t start, size_t end);
	BufferBlock getFirstBlock();
	BufferBlock getNextBlock(const BufferBlock& block);
	
private:
	char* _buffer;
	size_t _length;
	size_t _spareSize;
};

