#include "pipe.h"
#include <thread>

bool stopThread2 = false;

Pipe * Pipe::getInstance()
{
	static Pipe theOnlyPipe;
	return &theOnlyPipe;
}

void Pipe::bind(Device * a, Device * b)
{
	_pipeMutex.lock();
	_fromClientToServer.reset();
	_fromServerToClient.reset();
	_client = a;
	_server = b;
	stopThread2 = false;

	_pFromClientToServerThread = new std::thread(popSegmentHandler, &_fromClientToServer, _server);
	_pFromServerToClientThread = new std::thread(popSegmentHandler, &_fromServerToClient, _client);
}
void Pipe::releaseBind()
{
	if (stopThread2)
		return;
	stopThread2 = true;
	_pFromClientToServerThread->join();
	_pFromServerToClientThread->join();

	delete _pFromClientToServerThread;
	delete _pFromServerToClientThread;
	_pipeMutex.unlock();
}
Pipe::Pipe():
	_client(NULL),_server(NULL)
{
}
Pipe::~Pipe()
{
}
void Pipe::popSegmentHandler(SegmentController * controller, Device* target)
{
	while (1)
	{
		if (stopThread2)
			break;
		if (!controller->isEmpty())
		{
			segment seg;
			if(controller->pop(seg))
				target->getSegment(seg);
		}
	}
}
void Pipe::sendSegment(Device* sender, const segment& segment)
{
	if (sender == _client)
	{
		_fromClientToServer.push(segment);
	}
	else if (sender == _server)
	{
		_fromServerToClient.push(segment);
	}
}

void Pipe::getDevice(Device *& a, Device *& b)
{
	a = _client;
	b = _server;
}
