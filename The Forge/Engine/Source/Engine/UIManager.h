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

	private:
		static void Init();
		static void CleanUp();

		static void RenderWindows();
		static void FinishUIRender();
		
	private:

		static std::vector<UIWindow*> _uiWindows;

	};
}
