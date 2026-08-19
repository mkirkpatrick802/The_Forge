#include "GameEngine.h"

#include <fstream>
#include <SDL_timer.h>

#include "EngineManager.h"
#include "EventSystem.h"
#include "GameObject.h"
#include "InputManager.h"
#include "JsonKeywords.h"
#include "LaunchOptions.h"
#include "Level.h"
#include "LevelManager.h"
#include "NetworkManager.h"
#include "Rendering/Renderer.h"
#include "System.h"
#include "Time.h"
#include "../../../Netcode/Source/GamerServices.h"
#include "../../../Netcode/Source/Transport/UdpTransport.h"
#include "Collisions/CollisionManager.h"
#include "Components/ComponentManager.h"
#include "Components/SpriteRenderer.h"
#include "Editor/EditorManager.h"

Engine::GameEngine& Engine::GameEngine::GetInstance()
{
	static auto instance = std::make_unique<GameEngine>();
	return *instance;
}

Engine::GameEngine::GameEngine()
{
	_renderer = std::make_unique<Renderer>();
	_inputManager = std::make_unique<InputManager>();

	// Chat is an ImGui window driven by local key presses -- presentation only, and
	// there is no ImGui context when headless.
	if (!GetLaunchOptions().headless)
		_chat = std::make_unique<Chat>();
	else
		_console = std::make_unique<ServerConsole>();

	EventSystem::GetInstance()->RegisterEvent("Editor Enabled", this, &GameEngine::SceneStartup);
}

Engine::GameEngine::~GameEngine()
{
	_levelManager->CleanUp();

	EventSystem::GetInstance()->DeregisterEvent("Editor Enabled", this);
}

void Engine::GameEngine::ToggleLoadingScreen(bool enabled)
{
	// TODO: Finish this
	return;
	if (enabled)
	{
		_loadingScreen = std::make_unique<GameObject>();
		std::string filepath = "Assets/Loading Screen.prefab";
		if (!filepath.empty())
		{
			std::ifstream file(filepath);
			if (!file.is_open())
			{
				std::cerr << "Failed to open loading screen file " << filepath << '\n';
				return;
			}
        
			json j;
			file >> j; // Parse JSON from the file stream
			_loadingScreen->Deserialize(j);
			return;
		}
	}

	_loadingScreen.reset();
}

void Engine::GameEngine::SceneStartup(const void* p)
{
	if (const auto defaultData = GetEngineManager().GetConfigData(DEFAULTS_FILE, JsonKeywords::Config::DEFAULT_LEVEL); defaultData.is_string())
	{
		const std::string filename = defaultData;
		const std::string filepath = LEVEL_PATH + filename + ".json";
		if (_levelManager)
		{
			LevelManager::LoadLevel(filepath);
		}
		else
		{
			_levelManager = std::make_unique<LevelManager>(filepath);
		}
	}
	else
	{
		System::GetInstance().DisplayMessageBox("ERROR", "Could not load default level!!");
	}
}

void Engine::GameEngine::StartGameplayLoop()
{
	const bool headless = GetLaunchOptions().headless;

	if (!headless && !GetEngineManager().IsEditorEnabled())
		ToggleLoadingScreen(true);

	SceneStartup();

	if (headless)
		DEBUG_LOG("Scene loaded. Dedicated server running headless.")

	float frameStart = static_cast<float>(Time::GetTicks());
	if (!headless)
		_inputManager->ClearInputBuffers();

	// Headless has no window to pump events for, so the loop runs until something
	// asks the application to close rather than until the input manager says stop.
	while (headless ? !APPLICATION_CLOSING : _inputManager->StartProcessInputs())
	{
		if (const float currentTicks = static_cast<float>(Time::GetTicks()); currentTicks - frameStart >= 16)
		{
			const float deltaTime = (currentTicks - frameStart) / 1000.f;
			Time::SetDeltaTime(deltaTime);
			frameStart = currentTicks;
			
			if (!GetEngineManager().IsEditorEnabled())
			{
				GetCollisionManager().Update();

				// One call for every role. Whichever transport this process picked
				// is pumped from inside, so nothing here needs to know whether Steam,
				// UDP or nothing at all is underneath.
				NetCode::GetNetworkManager().Update();

				if (_chat)
					_chat->Update(deltaTime);

				if (_console)
					_console->Update();

				GetComponentManager().UpdateComponents(deltaTime);

				// Match rules tick after the world does, so a phase change reacts to the
				// state this frame produced rather than last frame's. A no-op on a client:
				// a level that arrived over the wire deliberately has no game mode.
				if (const Level* level = LevelManager::GetCurrentLevel())
					level->UpdateGameMode(deltaTime);
			}
			else
			{
				GetEditorManager().Update();
			}

#if DEBUG
			GetEngineManager().CollectDebugInputs();			
#endif
			
			if (!headless)
			{
				_renderer->Render();
				_inputManager->EndProcessInputs();
			}
		}
		else if (headless)
		{
			// Nothing to draw and no events to pump, so yield instead of burning a
			// core spinning on the frame-time check.
			SDL_Delay(1);
		}
	}
}
