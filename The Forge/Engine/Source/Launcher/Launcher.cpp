#include "Launcher.h"

#include "LauncherWindow.h"
#include "Engine/InputManager.h"
#include "Engine/Renderer.h"
#include "Engine/System.h"
#include "Engine/UIManager.h"

using namespace Engine;

void Launcher::Start()
{
	System::Init(Vector2D(300, 400));
	const auto _input = DEBUG_NEW InputManager();
	const auto _renderer = DEBUG_NEW Renderer();

	auto launcherUI = DEBUG_NEW LauncherWindow();
	UIManager::AddUIWindow(launcherUI);

	// Render UI
	while(_input->StartProcessInputs())
		_renderer->Render();

	delete launcherUI;
	launcherUI = nullptr;
}
