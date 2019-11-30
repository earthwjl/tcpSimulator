#include "pipe.h"

Pipe * Pipe::getInstance()
{
	static Pipe* onlyPipe = NULL;
	if (!onlyPipe)
		onlyPipe = new Pipe();
	return onlyPipe;
}

void Pipe::freePipe(Pipe * pipe)
{
	if (pipe)
		delete pipe;
}
Pipe::Pipe():
	_client(NULL),_server(NULL)
{
}