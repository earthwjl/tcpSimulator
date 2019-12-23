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
	void sendSegment(Device* sender,segment* segment);
	void getDevice(Device* & a, Device* & b);
private:
	Pipe();
	~Pipe();
	static void popSegmentHandler(SegmentController* controller, Device* target);
private:
	static bool _stopThread;
	Device* _client;
	Device* _server;
	std::mutex _pipeMutex;
	SegmentController _fromServerToClient;
	SegmentController _fromClientToServer;
	std::thread* _pFromServerToClientThread;
	std::thread* _pFromClientToServerThread;
};

