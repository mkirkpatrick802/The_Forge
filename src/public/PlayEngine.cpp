//
// Created by mKirkpatrick on 2/27/2024.
//

#include "PlayEngine.h"
#include <SDL_timer.h>
#include <SDL_scancode.h>

#include "InputManager.h"
#include "GameObjectManager.h"
#include "NetCode.h"
#include "RespawnManager.h"
#include "UIManager.h"

PlayEngine::PlayEngine(Renderer &renderer, InputManager &inputManager, NetCode* netcode) : Engine(renderer, inputManager)
{
    _netcode = netcode;
	_uiManager->AddUIWindow(&_leaderboards);
}

void PlayEngine::GameLoop()
{
	_gameObjectManager->LoadLevel();


	float frameStart = (float)SDL_GetTicks64();

	// Gameplay Loop
	while (_inputManager->StartProcessInputs(*_renderer))
	{
		if (float currentTicks = (float)SDL_GetTicks64(); currentTicks - frameStart >= 16)
		{
			// Frame Rate Management
			float deltaTime = (currentTicks - frameStart) / 1000.f;
			frameStart = currentTicks;

			if (_inputManager->GetKey(SDL_SCANCODE_APOSTROPHE))
				printf("%f FPS \n", 1.f / deltaTime);

			if (_inputManager->GetKey(SDL_SCANCODE_SEMICOLON))
				printf("%f DeltaTime \n", deltaTime);

			_netcode->Update(deltaTime);
			_gameObjectManager->Update(deltaTime);

			// Render UI
			_uiManager->Render();

			// Render game objects
			_renderer->Render();

			RespawnManager::GetInstance()->Update(deltaTime);

			// End input poll
			_inputManager->EndProcessInputs();
		}
	}
}

void PlayEngine::CleanUp()
{
    _gameObjectManager->CleanUp();
	RespawnManager::CleanUp();
}
