#pragma once
#include <cassert>
#include <type_traits>
#include <vector>

/*
 *  Inherit this class if you wish to notify other class of events, objects that inherit the event listener class need attached to this class to be able to receive events
 */

class Event;
class EventListener;

class EventDispatcher
{
public:

	virtual ~EventDispatcher() = default;

    void Attach(EventListener* listener);

    void Detach(EventListener* listener);

    void Notify(Event* event);

    template<typename T>
    std::enable_if_t<std::is_base_of_v<Event, T>, T*>
	CreateEvent();

    void CleanActiveEvents();

protected:

    std::vector<EventListener*> listeners;
    std::vector<Event*> activeEvents;

};

template <typename T>
std::enable_if_t<std::is_base_of_v<Event, T>, T*>
EventDispatcher::CreateEvent()
{
    auto newEvent = new T();

    assert(newEvent);

    activeEvents.push_back(newEvent);
    return newEvent;
}
