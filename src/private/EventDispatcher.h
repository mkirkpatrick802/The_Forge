#pragma once
#include "EventListener.h"

class EventDispatcher
{

public:

    void Attach(EventListener* listener) 
    {
        listeners.push_back(listener);
    }

    void Detach(EventListener* listener) 
    {
        std::erase(listeners, listener);
    }

    void Notify(EventType event) const
    {
        for (EventListener* listener : listeners) 
            listener->OnEvent(event);
        
    }


protected:

    std::vector<EventListener*> listeners;

};