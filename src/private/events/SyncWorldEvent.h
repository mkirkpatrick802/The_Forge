#pragma once
#include "Events.h"

class SyncWorldEvent : public Event
{
public:
	SyncWorldEvent() { _eventType = EventType::ET_SpawnPlayer; }

	const char* worldState;
};