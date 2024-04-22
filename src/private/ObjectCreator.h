#pragma once
#include "ComponentPool.h"
#include "GameData.h"
#include "PlayerController.h"
#include "nlohmann/json.hpp"

class Renderer;
using json = nlohmann::json;

class ObjectCreator;
class GameObject;

typedef void (ObjectCreator::* componentFn)(GameObject* go, const json& data);

class ObjectCreator
{
public:

    ObjectCreator(Renderer* renderer, InputManager* inputManager);

    GameObject* CreateGameObjectFromJSON(const json& gameObjects);
    void CreateComponentFromJSON(GameObject* go, const json& component);

    bool AddComponent(GameObject* go, glm::uint32 componentID);

    void UpdateComponentPools(float deltaTime);

    void CleanUpComponents(GameObject* go);

private:

    //Component Creation
    void RegisterComponentFns();
    void CreateSpriteRenderer(GameObject* go, const json& data);
    void CreatePlayerController(GameObject* go, const json& data);

private:

    // References
    Renderer* _renderer;
    InputManager* _inputManager;

    std::unordered_map<uint32, componentFn> componentCreationMap;

    ComponentPool<PlayerController> _playerControllerPool;

};