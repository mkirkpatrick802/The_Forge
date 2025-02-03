#include "GameEngine.h"

#include <SDL_keycode.h>
#include <SDL_mouse.h>

#include "EngineManager.h"
#include "EventSystem.h"
#include "InputManager.h"
#include "JsonKeywords.h"
#include "LevelManager.h"
#include "Rendering/Renderer.h"
#include "System.h"
#include "Time.h"
#include "Components/Component.h"
#include "Editor/EditorCamera.h"
#include "Rendering/UIManager.h"

Engine::GameEngine* Engine::GameEngine::_instance = nullptr;

Engine::GameEngine* Engine::GameEngine::GetInstance()
{
	return _instance ? _instance : DEBUG_NEW GameEngine();
}

Engine::GameEngine::GameEngine()
{
	_renderer = DEBUG_NEW Renderer();
	_inputManager = std::make_shared<InputManager>();

	const auto defaultData = EngineManager::GetConfigData(Engine::DEFAULTS_FILE, JsonKeywords::Config::DEFAULT_LEVEL);
	if (defaultData.is_string())
	{
		const std::string filename = defaultData;
		const std::string filepath = LEVEL_PATH + filename + ".json";
		_levelManager = DEBUG_NEW LevelManager(filepath);
	}
	else
	{
		System::DisplayMessageBox("ERROR", "Could not load default level!!");
	}

	_chat = std::make_unique<Chat>(_inputManager);
}

void Engine::GameEngine::StartGamePlayLoop()
{
	float frameStart = Time::GetTicks();

	_inputManager->ClearInputBuffers();
	while (_inputManager->StartProcessInputs())
	{
		if (const float currentTicks = Time::GetTicks(); currentTicks - frameStart >= 16)
		{
			const float deltaTime = (currentTicks - frameStart) / 1000.f;
			frameStart = currentTicks;

			EngineManager::UpdateNetObject();
			
			if (!EngineManager::IsEditorEnabled())
			{
				_chat->Update(deltaTime);
				
				// TODO: Call update function on component pools
			}
			
			_renderer->Render();

			_inputManager->EndProcessInputs();
		}
	}
}

void Engine::GameEngine::CleanUp()
{
	_levelManager->CleanUp();
	
	delete _levelManager;
	_levelManager = nullptr;

	_inputManager.reset();

	delete _renderer;
	_renderer = nullptr;
}
