//
// Created by mKirkpatrick on 1/30/2024.
//

#ifndef THE_FORGE_GAMEOBJECTMANAGER_H
#define THE_FORGE_GAMEOBJECTMANAGER_H

#include <unordered_map>
#include <vector>
#include "ComponentPool.h"
#include "PlayerController.h"
#include "GameData.h"

const std::string LEVEL_FILE = "assets/game-data/level.json";
const std::string PLAYER_FILE = "assets/game-data/player.json";

using json = nlohmann::json;

class GameObject;
class GameObjectManager;
class Renderer;
class InputManager;

typedef void (GameObjectManager::*componentFn)(GameObject* go, const json& data);

class GameObjectManager {

public:

    GameObjectManager();

    GameObjectManager(Renderer* renderer, InputManager* inputManager);

    // File Serialization
    void LoadLevel();
    void CreateGameObjectFromJSON(const json& gameObjects);
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

	void SpawnPlayer();

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

    std::vector<GameObject*> _currentGameObjects;
    ComponentPool<PlayerController> _playerControllerPool;

};


#endif //THE_FORGE_GAMEOBJECTMANAGER_H
