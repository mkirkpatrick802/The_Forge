#include "GameEngine.h"

#include <SDL_keycode.h>

#include "CommandTerminal.h"
#include "InputManager.h"
#include "LevelManager.h"
#include "Renderer.h"
#include "System.h"
#include "Time.h"
#include "UIManager.h"

Engine::GameEngine* Engine::GameEngine::_instance = nullptr;

Engine::GameEngine* Engine::GameEngine::GetInstance()
{
	return _instance ? _instance : DEBUG_NEW GameEngine();
}

Engine::GameEngine::GameEngine()
{
	_renderer = DEBUG_NEW Renderer();
	_inputManager = DEBUG_NEW InputManager();
	_levelManager = DEBUG_NEW LevelManager("path");
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

			if (_inputManager->GetKeyDown(SDL_SCANCODE_T))
				OpenCommandTerminal();

			if (_inputManager->GetKeyDown(SDL_SCANCODE_ESCAPE))
				CloseCommandTerminal();
			
			_inputManager->EndProcessInputs();
		}
	}
}

void Engine::GameEngine::OpenCommandTerminal()
{
	if (_terminal != nullptr) return;
	
	_terminal = DEBUG_NEW CommandTerminal();
	UIManager::AddUIWindow(_terminal);
}

void Engine::GameEngine::CloseCommandTerminal()
{
	if (_terminal == nullptr) return;
	
	UIManager::RemoveUIWindow(_terminal);
	delete _terminal;
	_terminal = nullptr;
}

void Engine::GameEngine::CleanUp()
{
	CloseCommandTerminal();
	_levelManager->CleanUp();
	
	delete _levelManager;
	_levelManager = nullptr;
	
	delete _inputManager;
	_inputManager = nullptr;

	delete _renderer;
	_renderer = nullptr;
}
