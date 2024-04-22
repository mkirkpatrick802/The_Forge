#pragma once

#include <vector>

#include "EventListener.h"
#include "GameData.h"
#include "PrefabManager.h"

class ObjectCreator;
const std::string LEVEL_FILE = "assets/game-data/level.json";

class GameObject;
class GameObjectManager;
class Renderer;
class InputManager;

class GameObjectManager : public EventListener
{
public:

    static void Init(Renderer* renderer, InputManager* inputManager);
    static GameObjectManager* GetInstance();

    void LoadLevel();

	GameObject* CreateGameObject(PrefabID ID);

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

	uint8 GenerateUniqueInstanceID();
	GameObject* GetGameObjectByInstanceID(uint8 ID);
    std::vector<GameObject*> GetCurrentGameObjects() const { return _currentGameObjects; }


private:

    GameObjectManager(Renderer* renderer, InputManager* inputManager);

private:

    static GameObjectManager* _instance;

	Renderer* _renderer;
	InputManager* _inputManager;
    ObjectCreator* _objectCreator;

	std::vector<GameObject*> _currentGameObjects;
};