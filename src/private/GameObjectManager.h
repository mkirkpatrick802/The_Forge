#pragma once

#include <unordered_map>
#include <vector>

#include "ComponentPool.h"
#include "EventListener.h"
#include "PlayerController.h"
#include "GameData.h"
#include "PrefabManager.h"

const std::string LEVEL_FILE = "assets/game-data/level.json";

using json = nlohmann::json;

class GameObject;
class GameObjectManager;
class Renderer;
class InputManager;

typedef void (GameObjectManager::*componentFn)(GameObject* go, const json& data);

class GameObjectManager : public EventListener
{
public:

    GameObjectManager(Renderer* renderer, InputManager* inputManager);

    // File Serialization
    void LoadLevel();
    GameObject* SpawnPrefab(const PrefabPath& path);
    GameObject* CreateGameObjectFromJSON(const json& gameObjects);
    void CreateComponentFromJSON(GameObject* go, const json& component);

    // Game Object Creation
    GameObject* CreateGameObject();
    bool AddComponent(GameObject* go, glm::uint32 componentID);

    // Editor Usability
    std::vector<GameObject*>* GetClickedObjects(Vector2D mousePos);
    void ToggleEditorMode(bool inEditorMode);

    // Gameplay Functions
    void Update(float deltaTime);

    void CleanUp();
    bool SaveGameObjectInfo();
    void SavePlayerObjectInfo(const GameObject* player);

    // Events
    virtual void OnEvent(Event* event) override;

    // Replications
    static int GetNumOfReplicatedObjects();

    static void CreateWorldState(char* worldState);
    static void CreateObjectState(const GameObject* object, char* state);
	void ReadWorldState(const char* state);

private:

    //Component Creation
    void RegisterComponentFns();
    void CreateSpriteRenderer(GameObject* go, const json& data);
    void CreatePlayerController(GameObject* go, const json& data);

public:

    //Object Creation
    std::unordered_map<uint32, componentFn> componentCreationMap;

private:

    Renderer* _renderer;
    InputManager* _inputManager;

    static std::vector<GameObject*> _currentGameObjects;
    ComponentPool<PlayerController> _playerControllerPool;

};