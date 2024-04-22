#include "Engine.h"
#include "GameObjectManager.h"
#include "UIManager.h"

Engine::Engine(Renderer& renderer, InputManager& inputManager) : _renderer(&renderer), _inputManager(&inputManager), frameStart(0), deltaTime(0)
{
	_uiManager = UIManager::GetInstance();

	GameObjectManager::Init(&renderer, &inputManager);
	_gameObjectManager = GameObjectManager::GetInstance();
}