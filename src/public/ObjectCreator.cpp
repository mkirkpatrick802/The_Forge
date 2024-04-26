#include "ObjectCreator.h"

#include <cassert>
#include <sstream>

#include "Component.h"
#include "GameObject.h"
#include "GameObjectManager.h"
#include "PlayerController.h"
#include "Renderer.h"

ObjectCreator::ObjectCreator(Renderer* renderer, InputManager* inputManager)
{
    _renderer = renderer;
    _inputManager = inputManager;

    RegisterComponentFns();
}

void ObjectCreator::RegisterComponentFns()
{
    componentCreationMap[SPRITE_RENDERER] = &ObjectCreator::CreateSpriteRenderer;
    componentCreationMap[PLAYER_CONTROLLER] = &ObjectCreator::CreatePlayerController;
    componentCreationMap[PROJECTILE] = &ObjectCreator::CreateProjectile;
    componentCreationMap[COLLIDER] = &ObjectCreator::CreateCollider;
    componentCreationMap[HEALTH] = &ObjectCreator::CreateHealth;
}

GameObject* ObjectCreator::CreateGameObjectFromJSON(const json &gameObject)
{
    GameObject* go = new GameObject();
    if(go == nullptr)
        assert(0 && "Failed to Create Game Object");

    ReadGameObject(go, gameObject);

    auto& components = gameObject["Components"];
    for (const auto& component : components)
        CreateComponentFromJSON(go, component);

    go->ObjectCreated();
    return go;
}

void ObjectCreator::ReadGameObject(GameObject* go, const json& data)
{
    if (data.contains("Prefab ID"))
    {
        const int prefabID = data["Prefab ID"];
        go->_prefabID = prefabID;
    }

    const std::string name = data["Name"];
    go->name = name;

    const int isReplicated = data["Is Replicated"];
    go->isReplicated = (bool)isReplicated;

    if (go->isReplicated)
        go->instanceID = GameObjectManager::GetInstance()->GenerateUniqueInstanceID();

    const std::string position = data["Position"];
    std::istringstream iss(position);
    iss >> go->transform.position.x >> go->transform.position.y;
}

void ObjectCreator::CreateComponentFromJSON(GameObject* go, const json& component)
{
    const int componentID = component["ID"];
    if (const auto it = componentCreationMap.find(componentID); it == componentCreationMap.end())
        assert(0 && "Could not find valid component function");

    (this->*componentCreationMap[componentID])(go, component);
}

bool ObjectCreator::AddComponent(GameObject* go, glm::uint32 componentID)
{
    auto it = componentCreationMap.find(componentID);
    if (it == componentCreationMap.end())
        return false;

    (this->*componentCreationMap[componentID])(go, nullptr);
    return true;
}

void ObjectCreator::CreateSpriteRenderer(GameObject* go, const json& data)
{
    if (_renderer != nullptr)
        _renderer->CreateSpriteRenderer(go, data);
}

void ObjectCreator::CreatePlayerController(GameObject* go, const json& data)
{
    PlayerController* playerController = _playerControllerPool.New(go);
    playerController->SetInputManager(_inputManager);

    go->AddComponent(playerController);

    if (data != nullptr)
        playerController->LoadData(data);
}

void ObjectCreator::CreateProjectile(GameObject* go, const json& data)
{
    Projectile* projectile = _projectilePool.New(go);
    go->AddComponent(projectile);

    if (data != nullptr)
        projectile->LoadData(data);
}

void ObjectCreator::CreateCollider(GameObject* go, const json& data)
{
    Collider* collider = _colliderPool.New(go);
    go->AddComponent(collider);

    if (data != nullptr)
        collider->LoadData(data);
}

void ObjectCreator::CreateHealth(GameObject* go, const json& data)
{
    Health* health = _healthPool.New(go);
    go->AddComponent(health);

    if (data != nullptr)
        health->LoadData(data);
}

void ObjectCreator::UpdateComponentPools(float deltaTime)
{
    _playerControllerPool.Update(deltaTime);
    _projectilePool.Update(deltaTime);
    _colliderPool.Update(deltaTime);
}

void ObjectCreator::CleanUpComponents(GameObject* go)
{
    if (PlayerController* controller = go->GetComponent<PlayerController>())
        _playerControllerPool.Delete(controller);

    if (Projectile* projectile = go->GetComponent<Projectile>())
        _projectilePool.Delete(projectile);

    if (Collider* collider = go->GetComponent<Collider>())
        _colliderPool.Delete(collider);

    if (Health* health = go->GetComponent<Health>())
        _healthPool.Delete(health);

    _renderer->CleanUpSpriteRenderer(go);
}