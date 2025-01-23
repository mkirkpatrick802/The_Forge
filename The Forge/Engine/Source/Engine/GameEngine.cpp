#include "GameEngine.h"

#include <SDL_keycode.h>
#include <SDL_mouse.h>

#include "EngineManager.h"
#include "EventSystem.h"
#include "InputManager.h"
#include "LevelManager.h"
#include "Rendering/Renderer.h"
#include "System.h"
#include "Time.h"
#include "Editor/EditorCamera.h"
#include "Editor/EditorManager.h"
#include "Rendering/UIManager.h"

Engine::GameEngine* Engine::GameEngine::_instance = nullptr;

Engine::GameEngine* Engine::GameEngine::GetInstance()
{
	return _instance ? _instance : DEBUG_NEW GameEngine();
}

Engine::GameEngine::GameEngine()
{
	_renderer = DEBUG_NEW Renderer();
	_inputManager = DEBUG_NEW InputManager();
	_levelManager = DEBUG_NEW LevelManager("path"); // This file path should be read in from a config file
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
				
			_renderer->Render();
			
			_inputManager->EndProcessInputs();
		}
	}
}

void Engine::GameEngine::CleanUp()
{
	_levelManager->CleanUp();

	Component::ClearComponentNames();
	
	delete _levelManager;
	_levelManager = nullptr;
	
	delete _inputManager;
	_inputManager = nullptr;

	delete _renderer;
	_renderer = nullptr;
}
