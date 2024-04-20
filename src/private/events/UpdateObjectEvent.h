#pragma once
#include "Events.h"

class UpdateObjectEvent : public Event
{
public:
	UpdateObjectEvent() { _eventType = EventType::ET_UpdateObject; }
	const char* objectState;
};