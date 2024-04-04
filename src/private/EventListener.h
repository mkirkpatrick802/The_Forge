#pragma once

enum class EventType {
    ET_SpawnPlayer
};

class EventListener 
{
public:

	virtual ~EventListener() = default;
	virtual void OnEvent(EventType event) = 0;
};