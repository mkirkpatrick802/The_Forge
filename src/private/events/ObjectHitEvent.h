#pragma once
#include "Events.h"

class ObjectHitEvent : public Event
{
public:

	ObjectHitEvent(): hit(nullptr) { _eventType = EventType::ET_ObjectHit; }
	GameObject* hit;
};