#include "Launcher.h"
#include "LauncherWindow.h"
#include "Engine/InputManager.h"
#include "Engine/Renderer.h"
#include "Engine/System.h"
#include "Engine/UIManager.h"

using namespace Engine;

void Launcher::RunLauncher(LauncherWindow* window, LauncherSettings& settings)
{
	auto _input = DEBUG_NEW InputManager();
	auto _renderer = DEBUG_NEW Renderer();

	window->SetLauncherSettings(&settings);
	UIManager::AddUIWindow(window);

	FindProjects(settings);
	
	// Update Loop
	while(_input->StartProcessInputs())
	{
		_renderer->Render();
	}

	UIManager::RemoveUIWindow(window);
	
	delete _input;
	_input = nullptr;

	delete _renderer;
	_renderer = nullptr;
}

void Launcher::FindProjects(LauncherSettings& settings)
{
	settings.projects.emplace_back("Project-Steel");
}
