#pragma once
#include "segment.h"
#include "device.h"
#include <mutex>
#include "SegmentController.h"

class Pipe
{
public:
	static Pipe* getInstance();
	void bind(Device* a, Device* b);
	void releaseBind();
	void sendSegment(Device* sender,const segment& segment);
	void getDevice(Device* & a, Device* & b);
private:
	Pipe();
	~Pipe();
private:
	Device* _client;
	Device* _server;
	std::mutex _pipeMutex;
	static void popSegmentHandler(SegmentController* controller, Device* target);
	SegmentController _fromServerToClient;
	SegmentController _fromClientToServer;
	std::thread* _pFromServerToClientThread;
	std::thread* _pFromClientToServerThread;
};

