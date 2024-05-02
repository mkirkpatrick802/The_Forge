#pragma once
#include <memory>
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

		static void AddUIWindow(UIWindow* window);
		static void RemoveUIWindow(UIWindow* window);

	private:

		static std::vector<UIWindow*> _uiWindows;

	};
}
