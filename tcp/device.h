#pragma once
#include "Port.h"
#include <map>
#include <set>
#include "segment.h"

class Process;
class Device
{
public:
	Device();
	~Device();
	bool processBindPort(Process* process, PortID id);
	void processReleasePort(Process* process);
	Process* createProcess();
	void deleteProcess(Process* process);
	void sendSegment(Device* other, const segment& seg);
	void getSegment(const segment& seg);
private:
	std::map<PortID, Process*> _portProcessMap;
	std::map<PortID, Port*>	_portMap;
	std::set<Process*> _processSet;
};
