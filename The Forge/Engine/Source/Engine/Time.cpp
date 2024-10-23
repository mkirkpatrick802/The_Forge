#include "Time.h"

#include <SDL_timer.h>


float Engine::Time::GetTicks()
{
	return (float)SDL_GetTicks64();
}

