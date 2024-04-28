#pragma once
#include "Events.h"

class OnDestroyedEvent : public Event
{
public:

	OnDestroyedEvent() { _eventType = EventType::ET_OnDestroyed; }
};