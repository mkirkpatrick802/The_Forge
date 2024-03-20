//
// Created by mKirkpatrick on 1/22/2024.
//

#include "Engine.h"
#include "GameObjectManager.h"

Engine::Engine(Renderer& renderer, InputManager& inputManager) : _renderer(&renderer), _inputManager(&inputManager) {
    _gameObjectManager = new GameObjectManager(&renderer, &inputManager);
}
