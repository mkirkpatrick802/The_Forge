#pragma once
#include "Events.h"

class SpawnPlayerEvent : public Event
{
public:

	SpawnPlayerEvent() { _eventType = EventType::ET_SpawnPlayer; }

	uint8 playerID;
};