#pragma once
#include <vector>
#include "UIWindow.h"

namespace Engine
{
	class UIManager
	{
	public:

		static void Init();

		static void RenderWindows();
		static void FinishUIRender();

		static void CleanUp();

		static void AddUIWindow(UIWindow* window) { _uiWindows.push_back(window); }
		static void RemoveUIWindow(UIWindow* window) { std::erase(_uiWindows, window); }

	private:

		static std::vector<UIWindow*> _uiWindows;

	};
}
