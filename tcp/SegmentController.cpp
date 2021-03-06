#include "SegmentController.h"
#include <iostream>
#include <thread>


class SegmentControlData
{
public:
	SegmentControlData(segment* segment);
	segment* _theSegment;
	bool	_setWrongData;
	unsigned int _setTimeInterval;
	bool	_setWrongOrder;
};
SegmentControlData::SegmentControlData(segment* segment):
	_theSegment(segment),_setWrongData(false),_setWrongOrder(false),_setTimeInterval(0)
{
}

SegmentController::SegmentController()
{
	_wrongDataRate = 0.0;
	_wrongOrderRate = 0.0;
	_maxTimeInterval = 100;
}


SegmentController::~SegmentController()
{
}

void SegmentController::push(segment * seg)
{
	float wrongData = rand() / (float)RAND_MAX;
	SegmentControlData* controlData = new SegmentControlData(seg);
	if (wrongData < _wrongDataRate)
		controlData->_setWrongData = true;
	float wrongOrder = rand() / (float)RAND_MAX;
	if (wrongOrder < _wrongOrderRate)
		controlData->_setWrongOrder = true;
	unsigned int timeInterval = rand() % _maxTimeInterval;
	controlData->_setTimeInterval = timeInterval;
	_theQueue.push_back(controlData);
}
bool SegmentController::isEmpty()const
{
	return _theQueue.empty();
}
bool SegmentController::pop(segment* & seg)
{
	if (_theQueue.empty())
		return false;
	SegmentControlData* controlData = _theQueue.front();
	if (controlData->_setWrongData)
	{
		shuffleSegmentData(controlData->_theSegment);
		controlData->_setWrongData = false;
	}
	if (controlData->_setWrongOrder)
	{
		shuffleSegmentOrder();
		controlData->_setWrongOrder = false;
	}
	controlData = _theQueue.front();

	if (controlData->_setTimeInterval > 0)
		std::this_thread::sleep_for(std::chrono::milliseconds(controlData->_setTimeInterval));

	seg = controlData->_theSegment;
	delete controlData;
	_theQueue.pop_front();
	return true;
}
void SegmentController::reset()
{
	for (SegmentControlData* segmentWrapped : _theQueue)
		delete segmentWrapped;
	_theQueue.clear();
}

void SegmentController::shuffleSegmentData(segment * seg)
{
	int count = rand() % 20;
	char* start = (char*)(seg);
	for (int i = 0; i < count; ++i)
	{
		int id = rand() % sizeof(segment);
		start[id] = 'r';
	}
}

void SegmentController::shuffleSegmentOrder()
{
	int index = rand() % _theQueue.size() - 1;
	std::list<SegmentControlData*>::iterator iter = _theQueue.begin();
	while (index--)
	{
		iter++;
	}
	SegmentControlData * tmp = *iter;
	*iter = _theQueue.front();
	*(_theQueue.begin()) = tmp;
}


