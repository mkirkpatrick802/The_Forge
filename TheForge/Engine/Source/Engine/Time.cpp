#include "Time.h"
#include <SDL_timer.h>

float Engine::Time::deltaTime = 0;

uint64_t Engine::Time::GetTicks()
{
	return SDL_GetTicks64();
}

float Engine::Time::GetDeltaTime()
{
	return deltaTime;
}

