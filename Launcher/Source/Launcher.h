#pragma once
#include <memory>
#include <string>
#include <vector>
#include <glm/vec2.hpp>

#include "LauncherWindow.h"
#include "Engine/System.h"

struct LauncherSettings
{
	std::vector<std::string> projects;

	void CleanUp()
	{
		projects.clear();
		projects.shrink_to_fit();
	}
};

class Launcher
{
public:
	
	template <typename TWindow>
	static void Start(LauncherSettings& settings, const glm::vec2& size);

private:

	static void RunLauncher(const std::shared_ptr<LauncherWindow>& window, LauncherSettings& settings);
	static void FindProjects(LauncherSettings& settings);
};

template <typename TWindow>
void Launcher::Start(LauncherSettings& settings, const glm::vec2& size)
{
	static_assert(std::is_base_of_v<LauncherWindow, TWindow>, "TWindow must be a subclass of LauncherWindow");
	Engine::CreateAppWindow(size);

	// UIManager owns UI windows through shared_ptr, so the window's lifetime is
	// shared rather than deleted here.
	RunLauncher(std::make_shared<TWindow>(), settings);
}