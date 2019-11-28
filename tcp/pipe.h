#pragma once
#include "segment.h"
class pipe
{
	friend class tcp;
public:
	pipe(device* d1,device* d2);
	~pipe();
	void read(segment& seg);
	void send(const segment& seg);
};

