#pragma once
#include <cstdint>

namespace Engine
{
	class Time
	{
	public:

		// Number of Ticks Since Engine Initialization
		static uint64_t GetTicks();

	};
}
