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

		static void AddUIWindow(const std::shared_ptr<UIWindow>& window);
		static void RemoveUIWindow(const std::shared_ptr<UIWindow>& window);
		static void RemoveAllUIWindows();

		static void SetDockingEnabled(const bool enabled) {_isDockingEnabled = enabled;}
		
	private:
		
		static void Init();
		static void CleanUp();

		static void CheckForRemovals();
		static void RenderWindows();
		static void FinishUIRender();
		
	private:

		static bool _isDockingEnabled;

		// bool represents if the ui window should be taken out of the vector.
		// So if bool = true then next frame it will be removed
		static std::vector<std::pair<bool, std::shared_ptr<UIWindow>>> _uiWindows;

	};
}
