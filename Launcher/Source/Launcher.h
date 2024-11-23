#pragma once
#include <string>
#include "LauncherWindow.h"
#include "../../The Forge/Engine/Source/Engine/Data.h"
#include "../../The Forge/Engine/Source/Engine/System.h"

struct LauncherSettings
{
	
};

class Launcher
{
public:
	
	template <typename TWindow>
	static void Start(LauncherSettings& settings, const Vector2D& size);

private:

	static void RunLauncher(LauncherWindow* window, LauncherSettings& settings);

};

template <typename TWindow>
void Launcher::Start(LauncherSettings& settings, const Vector2D& size)
{
	static_assert(std::is_base_of_v<LauncherWindow, TWindow>, "TWindow must be a subclass of LauncherWindow");
	Engine::System::CreateAppWindow(size);

	auto window = DEBUG_NEW TWindow();
	RunLauncher(window, settings);
	delete window;
}