#include "Engine/GameEngine.h"
#include "Launcher/Launcher.h"

void StartApplication();

int main()
{
	Engine::System::Init();

	StartApplication();

	Engine::System::CleanUp();
}

void StartApplication()
{
	// Create Launcher w/ settings
	// TODO CLEAN UP AFTER START
	LauncherSettings settings;
	Launcher::Start<LauncherWindow>(settings, Vector2D(300, 400));

	if (settings.mode != Engine::Mode::Null)
	{
		//Engine::EngineManager::GetInstance()->mode = settings.mode;

		Engine::System::CreateAppWindow();
		auto engine = Engine::GameEngine::GetInstance();
		engine->StartGamePlayLoop();
		Engine::GameEngine::CleanUp();

		//Engine::EngineManager::CleanUp();
	}
}