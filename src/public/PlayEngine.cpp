//
// Created by mKirkpatrick on 2/27/2024.
//

#include "PlayEngine.h"
#include <SDL_timer.h>
#include <SDL_scancode.h>
#include "InputManager.h"
#include "GameObjectManager.h"
#include "NetCode.h"

PlayEngine::PlayEngine(Renderer &renderer, InputManager &inputManager, NetCode* netcode) : Engine(renderer, inputManager)
{
    _netcode = netcode;
}

void PlayEngine::GameLoop()
{

    // Gameplay Loop
    while (_inputManager->StartProcessInputs(*_renderer, false))
    {
        if(SDL_GetTicks64() - frameStart >= 16)
        {
            //Frame Rate Management
            deltaTime = (SDL_GetTicks64() - frameStart);
            frameStart = SDL_GetTicks64();

            // Display frame rate in console
            if(_inputManager->GetKey(SDL_SCANCODE_APOSTROPHE))
                printf("%f FPS \n", 1000 / deltaTime);

            _netcode->Update();
            //_gameObjectManager->Update(deltaTime);

            // Render game objects
            //_renderer->Render();

            // End input poll
            //_inputManager->EndProcessInputs();
        }
    }
}

void PlayEngine::CleanUp()
{
    _gameObjectManager->CleanUp();
    delete _gameObjectManager;
}
