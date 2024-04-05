#pragma once
#include "EventListener.h"

/*
 *  Inherit this class if you wish to notify other class of events, objects that inherit the event listener class need attached to this class to be able to receive events
 */
class EventDispatcher
{
public:

	virtual ~EventDispatcher() = default;

	void Attach(EventListener* listener) 
    {
        listeners.push_back(listener);
    }

    void Detach(EventListener* listener) 
    {
        std::erase(listeners, listener);
    }

    template<typename T>
    std::enable_if_t<std::is_base_of_v<Event, T>, T*>
    CreateEvent();

    void Notify(const Event* event)
    {
        for (EventListener* listener : listeners)
            if (listener->IsSubscribedToEvent(event->GetEventType()))
				listener->OnEvent(event);

        CleanActiveEvents();
    }

    void CleanActiveEvents()
    {
        for (const auto event : activeEvents) 
            delete event;

        activeEvents.clear();
    }

protected:

    std::vector<EventListener*> listeners;
    std::vector<Event*> activeEvents;

};

template <typename T>
std::enable_if_t<std::is_base_of_v<Event, T>, T*>
EventDispatcher::CreateEvent()
{
    auto newEvent = new T();
    activeEvents.push_back(newEvent);
    return newEvent;
}