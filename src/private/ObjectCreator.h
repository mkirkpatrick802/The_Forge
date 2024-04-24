#pragma once

#include "Collider.h"
#include "GameData.h"
#include "nlohmann/json.hpp"

#include "ComponentPool.h"
#include "PlayerController.h"
#include "Projectile.h"

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

    void ReadGameObject(GameObject* go, const json& data);

    bool AddComponent(GameObject* go, uint32 componentID);

    void UpdateComponentPools(float deltaTime);

    void CleanUpComponents(GameObject* go);

private:

    //Component Creation
    void RegisterComponentFns();
    void CreateSpriteRenderer(GameObject* go, const json& data);
    void CreatePlayerController(GameObject* go, const json& data);
    void CreateProjectile(GameObject* go, const json& data);
    void CreateCollider(GameObject* go, const json& data);

private:

    // References
    Renderer* _renderer;
    InputManager* _inputManager;

    std::unordered_map<uint32, componentFn> componentCreationMap;

    ComponentPool<PlayerController> _playerControllerPool;
    ComponentPool<Projectile> _projectilePool;
    ComponentPool<Collider> _colliderPool;
};