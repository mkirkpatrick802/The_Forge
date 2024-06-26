#pragma once
#include <thread>


class UIManager;
class Renderer;
class InputManager;
class GameObjectManager;

class Engine
{
public:

	virtual ~Engine() = default;
	Engine(Renderer &renderer, InputManager &inputManager);

    virtual void GameLoop() = 0;
    virtual void CleanUp() = 0;

protected:

    // References
    Renderer* _renderer;
    UIManager* _uiManager;
    InputManager* _inputManager;
    GameObjectManager* _gameObjectManager;
};