#pragma once
class Buffer
{
public:
	Buffer(size_t len);
	virtual ~Buffer();
	size_t length()const { return _length; }
	bool isFull()const;
	bool isEmpty()const;

	char* read(size_t start, size_t end);
	char* read(char* buf, size_t len);
	void setSpare(char* buf, size_t len);
	void write(char* buf, size_t len);
	
private:
	char* _buffer;
	size_t _length;
	char* _firstSpareBlock;
	char* _firstOccupiedBlock;
};

