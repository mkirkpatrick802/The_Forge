#include "Engine.h"
#include "GameObjectManager.h"
#include "UIManager.h"

Engine::Engine(Renderer& renderer, InputManager& inputManager) : _renderer(&renderer), _inputManager(&inputManager)
{
	_uiManager = UIManager::GetInstance();

	GameObjectManager::Init(&renderer, &inputManager);
	_gameObjectManager = GameObjectManager::GetInstance();
}