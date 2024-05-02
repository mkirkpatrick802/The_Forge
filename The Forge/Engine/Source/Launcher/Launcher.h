#pragma once

#include "LauncherWindow.h"
#include "Engine/EngineManager.h"
#include "Engine/System.h"

struct LauncherSettings
{
	// Settings
	Engine::Mode mode = Engine::Mode::Null;
	std::string address{ "127.0.0.1" };
	bool isHosting = true;

	// Logic Guides
	bool playButtonPressed = false;
};

class Launcher
{
public:

	template <typename TWindow>
	static void Start(LauncherSettings& settings, Vector2D size);

private:

	static void RunLauncher(LauncherWindow* window, LauncherSettings& settings);

};

template <typename TWindow>
void Launcher::Start(LauncherSettings& settings, const Vector2D size)
{
	//static_assert(std::is_base_of_v<LauncherWindow, TWindow>, "TWindow must be a subclass of LauncherWindow");

	//Engine::System::CreateAppWindow(size);

	//auto window = DEBUG_NEW TWindow();
	//RunLauncher(window, settings);
	//delete window;
}