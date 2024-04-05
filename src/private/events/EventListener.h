#pragma once
#include "Events.h"

class EventListener 
{
public:

	void SubscribeToEvent(const EventType eventToAdd)
	{
		if (std::ranges::find(_subscribedEvents, eventToAdd) == _subscribedEvents.end())
			_subscribedEvents.push_back(eventToAdd);
	}

	void UnsubscribeToEvent(const EventType eventToRemove)
	{
		std::erase(_subscribedEvents, eventToRemove);
	}

	bool IsSubscribedToEvent(const EventType eventToCheck)
	{
		return std::ranges::find(_subscribedEvents, eventToCheck) != _subscribedEvents.end();
	}

	virtual ~EventListener() = default;
	virtual void OnEvent(const Event* event) = 0;

private:

	std::vector<EventType> _subscribedEvents;
};