//
// Created by mKirkpatrick on 1/22/2024.
//

#ifndef THE_FORGE_ENGINE_H
#define THE_FORGE_ENGINE_H

class Renderer;
class InputManager;
class GameObjectManager;

class Engine {
public:

    Engine(Renderer &renderer, InputManager &inputManager);

    virtual void GameLoop() = 0;
    virtual void CleanUp() = 0;

protected:

    // References
    Renderer* _renderer;
    InputManager* _inputManager;
    GameObjectManager* _gameObjectManager;

    // Time Keeping
    int frameStart;
    float deltaTime;
};


#endif //THE_FORGE_ENGINE_H
