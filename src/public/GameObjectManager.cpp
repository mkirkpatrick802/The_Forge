//
// Created by mKirkpatrick on 1/30/2024.
//

#include "GameObjectManager.h"
#include "GameObject.h"
#include <iostream>
#include <iomanip>

#include <fstream>

#include "DetailsChangedEvent.h"
#include "ObjectCreator.h"
#include "Renderer.h"
#include "PrefabManager.h"

GameObjectManager* GameObjectManager::_instance = nullptr;

void GameObjectManager::Init(Renderer* renderer, InputManager* inputManager)
{
    if (_instance) return;

    _instance = new GameObjectManager(renderer, inputManager);
}

GameObjectManager::GameObjectManager(Renderer* renderer, InputManager* inputManager) : _renderer(renderer), _inputManager(inputManager)
{
    SubscribeToEvent(EventType::ET_SpawnPlayer);
    SubscribeToEvent(EventType::ET_DetailsChanged);

    _objectCreator = new ObjectCreator(renderer, inputManager);

    std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

GameObjectManager* GameObjectManager::GetInstance()
{
    if (!_instance)
    {
        printf("Game Object Manager Not Init \n");
        return nullptr;
    }

    return _instance;
}

void GameObjectManager::LoadLevel()
{
    std::ifstream file(LEVEL_FILE);

    if (!file.is_open()) 
    {
        std::cerr << "Could not open file for reading!\n";
        assert(0);
    }

    json levelData;
    file >> levelData;
    file.close();

    auto& gameObjects = levelData["GameObjects"];
    for (const auto& gameObject : gameObjects)
    {
	    if (gameObject.contains("Prefab ID"))
	    {
            GameObject* go = CreateGameObject(gameObject["Prefab ID"]);
            _objectCreator->ReadGameObject(go, gameObject);
	    }
        else
        {
            _currentGameObjects.push_back(_objectCreator->CreateGameObjectFromJSON(gameObject));
        }
    }
}

GameObject* GameObjectManager::CreateGameObject(const PrefabID ID, Vector2D position, float rotation)
{
	if (_currentGameObjects.size() >= MAX_GAMEOBJECTS - 5)
		return nullptr;

    GameObject* go;
    if (ID == -1)
    {
        go = new GameObject();
    }
    else
    {
        const PrefabPath path = PrefabManager::GetInstance().GetPrefabPath(ID);
        std::ifstream file(path);

        if (!file.is_open())
        {
            std::cerr << "Could not open file for reading!\n";
            assert(0);
        }

        json prefab;
        file >> prefab;
        file.close();

        go = _objectCreator->CreateGameObjectFromJSON(prefab);
    }

    go->SetPosition(position);
    go->transform.rotation = rotation;

    _currentGameObjects.push_back(go);
    return go;
}

void GameObjectManager::Update(const float deltaTime)
{
    _objectCreator->UpdateComponentPools(deltaTime);
}

void GameObjectManager::DestroyGameObject(GameObject* go)
{
    for (auto it = _currentGameObjects.begin(); it != _currentGameObjects.end(); ++it)
        if (*it == go)
        {
            _objectCreator->CleanUpComponents(go);
            _currentGameObjects.erase(it);
            delete go;
            return;
        }
}

std::vector<GameObject*>* GameObjectManager::GetClickedObjects(Vector2D mousePos)
{
    auto clickedGameObjects = new std::vector<GameObject*>();
    for (auto go : _currentGameObjects)
        if(go->IsClicked(mousePos))
            clickedGameObjects->push_back(go);

    return clickedGameObjects;
}

void GameObjectManager::ToggleEditorMode(bool inEditorMode)
{
    if(inEditorMode) 
    {
        CleanUp();
        LoadLevel();
        return;
    }

	SaveGameObjectInfo();
}

bool GameObjectManager::SaveGameObjectInfo()
{
    printf("Saving Level Data! \n");
    std::ifstream in(LEVEL_FILE);

    if (!in.is_open()) {
        std::cerr << "Could not open file for reading!\n";
        return false;
    }

    json levelData;
    in >> levelData;
    in.close();

    for (int i = 0; i < _currentGameObjects.size(); i++) 
    {
        // TODO: Clean this up
        if (_currentGameObjects[i]->GetComponent<PlayerController>())
        {
            SavePlayerObjectInfo(_currentGameObjects[i]);
            continue;
        }

    	if(_currentGameObjects[i]->GetComponent<Projectile>())
            continue;

        // Check if prefab
        if (_currentGameObjects[i]->_prefabID != -1)
        {
            levelData["GameObjects"][i]["Prefab ID"] = (int)_currentGameObjects[i]->_prefabID;
        }

        levelData["GameObjects"][i]["Name"] = _currentGameObjects[i]->_name;
        levelData["GameObjects"][i]["Is Replicated"] = (int)_currentGameObjects[i]->isReplicated;
        levelData["GameObjects"][i]["Position"] = _currentGameObjects[i]->GetPositionString();
    }

    std::ofstream out(LEVEL_FILE);
    out << std::setw(2) << levelData << std::endl;
    out.close();

    printf("Saved Level Data! \n");
    return true;
}

void GameObjectManager::SavePlayerObjectInfo(const GameObject* player)
{
    printf("Saving Player Data! \n");
    std::ifstream in(PrefabManager::GetInstance().GetPrefabPath(PLAYER_PREFAB_ID));

    if (!in.is_open()) 
    {
        std::cerr << "Could not open file for reading!\n";
        assert(0);
    }

    json playerData;
    in >> playerData;
    in.close();

    playerData["Prefab ID"] = (int)player->_prefabID;
    playerData["Name"] = player->_name;
    playerData["Is Replicated"] = (int)player->isReplicated;
    playerData["Position"] = player->GetPositionString();

    std::ofstream out(PrefabManager::GetInstance().GetPrefabPath(PLAYER_PREFAB_ID));
    out << std::setw(2) << playerData << std::endl;
    out.close();

    printf("Saved Player Data! \n");
}

void GameObjectManager::OnEvent(Event* event)
{
    switch (event->GetEventType())
    {
    case EventType::ET_NULL:
        break;

    case EventType::ET_SpawnPlayer:
    	CreateGameObject(PLAYER_PREFAB_ID);
        break;

	case EventType::ET_DetailsChanged:
        ToggleEditorMode(static_cast<DetailsChangedEvent*>(event)->currentDetails.editorSettings.editMode); // VS says this static cast is bad IDK why (Explore later)
		break;
    }
}

uint8 GameObjectManager::GenerateUniqueInstanceID()
{
    // Create Unique ID
    bool IsUnique = false;
    uint8 newID;

    do
    {
        IsUnique = true;
        const int randomNumber = std::rand() % MAX_GAMEOBJECTS;
        newID = (uint8)randomNumber;
        for (const auto go : _currentGameObjects)
        {
            if (go->instanceID != newID) continue;
            IsUnique = false;
            break;
        }
    } while (!IsUnique);

    return newID;
}

GameObject* GameObjectManager::GetGameObjectByInstanceID(const uint8 ID)
{
	for (const auto go : _currentGameObjects)
        if (go->instanceID == ID)
            return go;

    return nullptr;
}

void GameObjectManager::CleanUp()
{
    const uint32 size = _currentGameObjects.size();
    for (int i = 0; i < size; ++i)
    {
        _objectCreator->CleanUpComponents(_currentGameObjects[i]);
        delete _currentGameObjects[i];
    }
    _currentGameObjects.clear();

    delete _objectCreator;
    delete _instance;
}
