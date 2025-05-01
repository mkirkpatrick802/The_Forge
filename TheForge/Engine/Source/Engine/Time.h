#pragma once
#include <cstdint>

namespace Engine
{
	class Time
	{
		friend class GameEngine;
	public:

		// Number of Ticks Since Engine Initialization
		static uint64_t GetTicks();
		static float GetDeltaTime();

	private:
		static void SetDeltaTime(const float delta) { deltaTime = delta; }
		static float deltaTime;
	};
}
