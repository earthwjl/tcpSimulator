#include "pipe.h"
#include <thread>
#include <iostream>


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
	_stopThread = false;

	_pFromClientToServerThread = new std::thread(popSegmentHandler, &_fromClientToServer, _server);
	_pFromServerToClientThread = new std::thread(popSegmentHandler, &_fromServerToClient, _client);
}
void Pipe::releaseBind()
{
	if (_stopThread)
		return;
	_stopThread = true;
	_pFromClientToServerThread->join();
	_pFromServerToClientThread->join();

	delete _pFromClientToServerThread;
	delete _pFromServerToClientThread;
	_pipeMutex.unlock();
}
Pipe::Pipe():
	_client(NULL),_server(NULL)
{
	if (_stopThread == false)
	{
		releaseBind();
	}
}
bool Pipe::_stopThread = true;
Pipe::~Pipe()
{
}
void Pipe::popSegmentHandler(SegmentController * controller, Device* target)
{
	while (1)
	{
		if (_stopThread)
			break;
		if (!controller->isEmpty())
		{
			controller->_lock.lock();
			segment seg;
			if (controller->pop(seg))
			{
				target->getSegment(seg);
			}
			controller->_lock.unlock();
		}
	}
}
void Pipe::sendSegment(Device* sender, const segment& segment)
{

	if (sender == _client)
	{
		std::cout << "pipe client send segment id " << segment.id << " ackid " << segment.bufferLength()+segment.id << std::endl;
		_fromClientToServer.push(segment);
	}
	else if (sender == _server)
	{
		std::cout << "pipe server send segment" << std::endl;
		_fromServerToClient.push(segment);
	}
}

void Pipe::getDevice(Device *& a, Device *& b)
{
	a = _client;
	b = _server;
}
