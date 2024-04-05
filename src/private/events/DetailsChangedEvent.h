#pragma once
#include "Events.h"
#include "UIStructs.h"

class DetailsChangedEvent : public Event
{
public:
	DetailsChangedEvent(): currentDetails() { _eventType = EventType::ET_DetailsChanged; }

	Details currentDetails;
};