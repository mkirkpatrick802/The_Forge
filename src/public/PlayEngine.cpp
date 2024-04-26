//
// Created by mKirkpatrick on 2/27/2024.
//

#include "PlayEngine.h"
#include <SDL_timer.h>
#include <SDL_scancode.h>

#include "InputManager.h"
#include "GameObjectManager.h"
#include "NetCode.h"
#include "UIManager.h"

PlayEngine::PlayEngine(Renderer &renderer, InputManager &inputManager, NetCode* netcode) : Engine(renderer, inputManager)
{
    _netcode = netcode;
	_uiManager->AddUIWindow(&_leaderboards);
}

void PlayEngine::GameLoop()
{
	_gameObjectManager->LoadLevel();

	// Gameplay Loop
	while (_inputManager->StartProcessInputs(*_renderer))
	{
		if (SDL_GetTicks64() - frameStart >= 16)
		{
			//Frame Rate Management
			deltaTime = (SDL_GetTicks64() - frameStart) / 1000.f;
			frameStart = SDL_GetTicks64();

			// Display frame rate in console
			if (_inputManager->GetKey(SDL_SCANCODE_APOSTROPHE))
				printf("%f FPS \n", 1000.f / (deltaTime * 1000));

			_netcode->Update(deltaTime);
			_gameObjectManager->Update(deltaTime);

			// Render UI
			_uiManager->Render();

			// Render game objects
			_renderer->Render();

			// End input poll
			_inputManager->EndProcessInputs();
		}
	}
}

void PlayEngine::CleanUp()
{
    _gameObjectManager->CleanUp();
}
