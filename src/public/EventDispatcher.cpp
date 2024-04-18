#include "EventDispatcher.h"

#include "EventListener.h"

void EventDispatcher::Attach(EventListener* listener)
{
	listeners.push_back(listener);
}

void EventDispatcher::Detach(EventListener* listener)
{
    std::erase(listeners, listener);
}

void EventDispatcher::Notify(Event* event)
{
    for (EventListener* listener : listeners)
        if (listener->IsSubscribedToEvent(event->GetEventType()))
            listener->OnEvent(event);

    CleanActiveEvents();
}

void EventDispatcher::CleanActiveEvents()
{
    for (const auto event : activeEvents)
        delete event;

    activeEvents.clear();
}