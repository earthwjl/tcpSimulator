#pragma once
#include "pipe.h"

class device
{
public:
	device();
	~device();
	bool bind(device* other);
	void read(char* buf,size_t& len);
	void write(char* buf, size_t len);
	bool release();
private:
	device* communicator;
	bool isbinded;
	pipe* thePipe;
};

