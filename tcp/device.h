#pragma once
#include "Port.h"
#include <map>
#include <set>
#include "segment.h"

class Process;
class Pipe;
class Device
{
public:
	Device();
	~Device();
	bool processBindPort(Process* process, short id);
	void processReleasePort(Process* process);
	Process* createProcess();
	void deleteProcess(Process* process);
	Port* getPort(short id);
	Process* getBindedProcess(short portId);
	void sendSegment(segment* seg);
	void getSegment(const segment* seg);
private:
	std::map< short, Process*> _portProcessMap;
	std::map< short, Port*>	_portMap;
	std::set<Process*> _processSet;
};
