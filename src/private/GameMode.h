#pragma once
#include "EventDispatcher.h"

class GameMode : public EventDispatcher
{
public:

	void BeginPlay();
	void Update(float deltaTime);
};
