#pragma once
#include "segment.h"
class sender
{
public:
	sender();
	~sender();
	size_t windowSize();
	void send();
private:
	segment* buffer[1024];
	char* swindow_left;
	char* sended;
	char* swindow_right;
};
class writer
{

};

