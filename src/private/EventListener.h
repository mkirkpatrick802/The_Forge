#pragma once

enum class EventType
{
	ET_SpawnPlayer,
	ET_DetailsChanged
};

class EventListener 
{
public:

	virtual ~EventListener() = default;
	virtual void OnEvent(EventType event) = 0;
};