#pragma once
#include "EventTypes.h"

class Event
{
public:
	Event() { _eventType = EventType::ET_NULL; }

	EventType GetEventType() const { return _eventType; }

protected:

	EventType _eventType;
};