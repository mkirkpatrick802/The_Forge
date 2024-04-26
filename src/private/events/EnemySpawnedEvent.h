#pragma once
#include "Events.h"

class GameObject;
class EnemySpawnedEvent : public Event
{
public:

	EnemySpawnedEvent(): spawned(nullptr) { _eventType = EventType::ET_EnemySpawned; }
	GameObject* spawned;
};