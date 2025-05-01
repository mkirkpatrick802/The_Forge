#include "GameEngine.h"

#include <fstream>

#include "EngineManager.h"
#include "EventSystem.h"
#include "GameObject.h"
#include "InputManager.h"
#include "JsonKeywords.h"
#include "LevelManager.h"
#include "NetworkManager.h"
#include "Rendering/Renderer.h"
#include "System.h"
#include "Time.h"
#include "../../../Netcode/Source/GamerServices.h"
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
	_chat = std::make_unique<Chat>();

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
	if (!GetEngineManager().IsEditorEnabled())
		ToggleLoadingScreen(true);
	
	SceneStartup();
	
	float frameStart = static_cast<float>(Time::GetTicks());
	_inputManager->ClearInputBuffers();
	while (_inputManager->StartProcessInputs())
	{
		if (const float currentTicks = static_cast<float>(Time::GetTicks()); currentTicks - frameStart >= 16)
		{
			const float deltaTime = (currentTicks - frameStart) / 1000.f;
			Time::SetDeltaTime(deltaTime);
			frameStart = currentTicks;
			
			if (!GetEngineManager().IsEditorEnabled())
			{
				GetCollisionManager().Update();
				NetCode::GetGamerService().Update();
				NetCode::GetNetworkManager().Update();
				_chat->Update(deltaTime);
				GetComponentManager().UpdateComponents(deltaTime);
				//std::cout << 1 / deltaTime << '\n';
			}
			else
			{
				GetEditorManager().Update();
			}

#if DEBUG
			GetEngineManager().CollectDebugInputs();			
#endif
			
			_renderer->Render();
			_inputManager->EndProcessInputs();
		}
	}
}
