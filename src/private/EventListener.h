#pragma once

enum class EventType {
    ET_SpawnPlayer
};

class EventListener 
{
protected:

    virtual void OnEvent(EventType event) = 0;
};