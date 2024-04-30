#include "GameEngine.h"

#include <SDL_keycode.h>

#include "InputManager.h"
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
	System::Init();

	_renderer = DEBUG_NEW Renderer();
	_inputManager = DEBUG_NEW InputManager();
}

void Engine::GameEngine::StartGamePlayLoop() const
{
	float frameStart = Time::GetTicks();

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
	delete _inputManager;
	_inputManager = nullptr;

	delete _renderer;
	_renderer = nullptr;

	delete _instance;
	_instance = nullptr;

	System::CleanUp();
}