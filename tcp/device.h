#pragma once
#include "Process.h"
#include "reader.h"
#include "writer.h"
#include <iostream>
#include <map>

typedef unsigned short PortID;

class Device
{
	friend class Process;
public:
	Device();
	~Device();
	Process* createProcess();
	void releaseProcess(Process* process);
private:
	bool bindProcess(PortID port, Process* process);
private:
	Reader _reader;
	Writer _writer;
	std::map<PortID, Process*> _portMap;
};

