#include "Time.h"
#include <SDL_timer.h>

uint64_t Engine::Time::GetTicks()
{
	return SDL_GetTicks64();
}

