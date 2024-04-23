#include "ObjectCreator.h"

#include <cassert>
#include <sstream>

#include "Component.h"
#include "GameObject.h"
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
}

GameObject* ObjectCreator::CreateGameObjectFromJSON(const json &gameObject)
{
    GameObject* go = new GameObject();
    if(go == nullptr)
        assert(0 && "Failed to Create Game Object");

    // Check if prefab
    if(gameObject.find("Prefab ID") != gameObject.end())
    {
        const int prefabID = gameObject["Prefab ID"];
        go->_prefabID = prefabID;
    }

    const std::string name = gameObject["Name"];
    go->_name = name;

    const int isReplicated = gameObject["Is Replicated"];
    go->isReplicated = (bool)isReplicated;

    const std::string position = gameObject["Position"];
    std::istringstream iss(position);
    iss >> go->transform.position.x >> go->transform.position.y;

    auto& components = gameObject["Components"];
    for (const auto& component : components)
        CreateComponentFromJSON(go, component);

    go->ObjectCreated();
    return go;
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

void ObjectCreator::UpdateComponentPools(float deltaTime)
{
    _playerControllerPool.Update(deltaTime);
    _projectilePool.Update(deltaTime);
}

void ObjectCreator::CleanUpComponents(GameObject* go)
{
    if (PlayerController* controller = go->GetComponent<PlayerController>())
        _playerControllerPool.Delete(controller);

    _renderer->CleanUpSpriteRenderer(go);
}