#pragma once
#include "segment.h"
#include <list>
#include <mutex>
class SegmentControlData;
class SegmentController
{
public:
	SegmentController();
	~SegmentController();
	void setWrongDataRate(float rate) { _wrongDataRate = rate; }
	void setWrongOrderRate(float rate) { _wrongOrderRate = rate; }
	void setMaxTimeInterval(unsigned int ms) { _maxTimeInterval = ms; }
	void push(segment* seg);
	bool isEmpty()const;
	bool pop(segment* & seg);
	void reset();
	std::mutex _lock;

private:
	void shuffleSegmentData(segment* seg);
	void shuffleSegmentOrder();
private:
	std::list<SegmentControlData*> _theQueue;
	float _wrongDataRate;
	float _wrongOrderRate;
	unsigned int _maxTimeInterval;
};

