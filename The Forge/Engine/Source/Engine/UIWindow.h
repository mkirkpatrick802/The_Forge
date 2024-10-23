#pragma once
#include "imgui.h"

namespace Engine
{
	class UIWindow
	{
	public:

		virtual void Render() = 0;
		virtual ~UIWindow() = default;
	};
}