#include "GameEngine.h"

#include <SDL_keycode.h>

#include "InputManager.h"
#include "Renderer.h"
#include "Time.h"

Engine::GameEngine* Engine::GameEngine::_instance = nullptr;

Engine::GameEngine* Engine::GameEngine::GetInstance()
{
	if(!_instance)
		Init();

	return _instance;
}

void Engine::GameEngine::Init()
{
	System::Init();
	_instance = DEBUG_NEW GameEngine();
}

Engine::GameEngine::GameEngine()
{
	_renderer = DEBUG_NEW Renderer();
	_inputManager = DEBUG_NEW InputManager();
}

void Engine::GameEngine::StartGamePlayLoop()
{
	float frameStart = Time::GetTicks();

	while (_inputManager->StartProcessInputs())
	{
		if (float currentTicks = Time::GetTicks(); currentTicks - frameStart >= 16)
		{
			float deltaTime = (currentTicks - frameStart) / 1000.f;
			frameStart = currentTicks;

			if (_inputManager->GetKey(SDL_SCANCODE_W))
				printf("%f FPS \n", 1.f / deltaTime);


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