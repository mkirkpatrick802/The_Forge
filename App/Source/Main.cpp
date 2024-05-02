#include "Engine/GameEngine.h"
#include "Launcher/Launcher.h"

int main()
{
	Engine::System::Init();
	LauncherSettings settings;
	Launcher::Start<LauncherWindow>(settings, Vector2D(300, 400));

	/*if (settings.mode != Engine::Mode::Null)
	{
		// Set Engine Settings
		Engine::EngineManager::GetInstance()->mode = settings.mode;

		// Start Engine Loop
		auto engine = Engine::GameEngine::GetInstance();
		engine->StartGamePlayLoop();
		engine->CleanUp();

		Engine::EngineManager::CleanUp();
	}*/

	Engine::System::CleanUp();
}