#pragma once
#include "sender.h"
#include "segment.h"

class Reader
{
public:
private:
	bool decodeSegment(segment& seg);
private:
	char _buffer[4096];
};

