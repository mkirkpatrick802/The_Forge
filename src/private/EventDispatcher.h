#pragma once
#include "EventListener.h"

class EventDispatcher
{
public:

    void Attach(EventListener* listener) {
        listeners.push_back(listener);
    }

    void Detach(EventListener* listener) {
        listeners.erase(std::remove(listeners.begin(), listeners.end(), listener), listeners.end());
    }

    void Notify(EventType event) {
        for (EventListener* listener : listeners) {
            listener->OnEvent(event);
        }
    }


protected:

    std::vector<EventListener*> listeners;

};