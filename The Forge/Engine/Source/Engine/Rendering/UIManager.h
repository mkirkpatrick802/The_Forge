#pragma once
#include <memory>
#include <vector>
#include "UIWindow.h"

namespace Engine
{
	class UIManager
	{
		friend class Renderer;
	public:

		static void AddUIWindow(UIWindow* window);
		static void RemoveUIWindow(UIWindow* window);

		static void SetDockingEnabled(const bool enabled) {_isDockingEnabled = enabled;};
		
	private:
		
		static void Init();
		static void CleanUp();

		static void RenderWindows();
		static void FinishUIRender();
		
	private:

		static bool _isDockingEnabled;
		static std::vector<UIWindow*> _uiWindows;

	};
}
