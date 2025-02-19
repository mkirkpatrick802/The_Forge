#include "GameEngine.h"

#include "EngineManager.h"
#include "InputManager.h"
#include "JsonKeywords.h"
#include "LevelManager.h"
#include "NetworkManager.h"
#include "Rendering/Renderer.h"
#include "System.h"
#include "Time.h"
#include "../../../Netcode/Source/GamerServices.h"
#include "Components/ComponentManager.h"
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
}

Engine::GameEngine::~GameEngine()
{
	_levelManager->CleanUp();
}

void Engine::GameEngine::SceneStartup()
{
	if (const auto defaultData = GetEngineManager().GetConfigData(Engine::DEFAULTS_FILE, JsonKeywords::Config::DEFAULT_LEVEL); defaultData.is_string())
	{
		const std::string filename = defaultData;
		const std::string filepath = LEVEL_PATH + filename + ".json";
		_levelManager = std::make_unique<LevelManager>(filepath);
	}
	else
	{
		System::GetInstance().DisplayMessageBox("ERROR", "Could not load default level!!");
	}
}

void Engine::GameEngine::StartGameplayLoop()
{
	SceneStartup();
	
	float frameStart = (float)Time::GetTicks();

	_inputManager->ClearInputBuffers();
	while (_inputManager->StartProcessInputs())
	{
		if (const float currentTicks = (float)Time::GetTicks(); currentTicks - frameStart >= 16)
		{
			const float deltaTime = (currentTicks - frameStart) / 1000.f;
			frameStart = currentTicks;
			
			if (!GetEngineManager().IsEditorEnabled())
			{
				NetCode::GetGamerService().Update();
				NetCode::GetNetworkManager().Update();
				_chat->Update(deltaTime);
				GetComponentManager().UpdateComponents(deltaTime);
			}else{
				GetEditorManager().Update();
			}
			
			_renderer->Render();
			_inputManager->EndProcessInputs();
		}
	}
}
