#pragma once
#include "segment.h"
#include "device.h"

class Pipe
{
public:
	static Pipe* getInstance();
	static void freePipe(Pipe* pipe);
	void bind(Device* a, Device* b);
	void sendSegment(const segment& segment);
	void registerSegmentHandler(void(handler)(const segment& seg));
private:
	Pipe();
	~Pipe();
private:
	Device* client;
	Device* server;
};

