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
#include "SpawnPlayerEvent.h"

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

    LoadLevel();

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
        _currentGameObjects.push_back(_objectCreator->CreateGameObjectFromJSON(gameObject));
}

GameObject* GameObjectManager::CreateGameObject(const PrefabID ID, Vector2D position, float rotation)
{
    if (_currentGameObjects.size() >= MAX_GAMEOBJECTS)
        assert(0 && "To Many GameObjects");

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
    /*printf("Saving Level Data! \n");
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

        // Check if prefab
        if (_currentGameObjects[i]->_prefabID != -1)
        {
            levelData["GameObjects"][i]["Prefab ID"] = (int)_currentGameObjects[i]->_prefabID;
        }

        levelData["GameObjects"][i]["Name"] = _currentGameObjects[i]->_name;
        levelData["GameObjects"][i]["Is Replicated"] = (int)_currentGameObjects[i]->_isReplicated;
        levelData["GameObjects"][i]["Position"] = _currentGameObjects[i]->GetPositionString();
    }

    std::ofstream out(LEVEL_FILE);
    out << std::setw(2) << levelData << std::endl;
    out.close();

    printf("Saved Level Data! \n");
    return true;*/

    return false;
}

void GameObjectManager::SavePlayerObjectInfo(const GameObject* player)
{
    /*printf("Saving Player Data! \n");
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
    playerData["Is Replicated"] = (int)player->_isReplicated;
    playerData["Position"] = player->GetPositionString();

    std::ofstream out(PrefabManager::GetInstance().GetPrefabPath(PLAYER_PREFAB_ID));
    out << std::setw(2) << playerData << std::endl;
    out.close();

    printf("Saved Player Data! \n");*/
}

void GameObjectManager::OnEvent(Event* event)
{
    switch (event->GetEventType())
    {
    case EventType::ET_NULL:
        break;

    case EventType::ET_SpawnPlayer:
	    {
			/*const auto player = SpawnPrefab(PrefabManager::GetInstance().GetPrefabPath(PLAYER_PREFAB_ID));
			const int ID = static_cast<SpawnPlayerEvent*>(event)->playerID;
			player->GetComponent<PlayerController>()->InitController(ID);*/
	    }
        break;

	case EventType::ET_DetailsChanged:
        ToggleEditorMode(static_cast<DetailsChangedEvent*>(event)->currentDetails.editorSettings.editMode); // VS says this static cast is bad IDK why (Explore later)
		break;

	/*case EventType::ET_SyncWorld:
        ReadWorldState(static_cast<SyncWorldEvent*>(event)->worldState);
		break;
	case EventType::ET_UpdateObject:
        ReadObjectState(static_cast<UpdateObjectEvent*>(event)->objectState);*/
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
        for (const auto go : GameObjectManager::_currentGameObjects)
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

/*
 *      Game State Replication
 */

/*int GameObjectManager::GetNumOfReplicatedObjects()
{
    int num = 0;
    for (const auto go : _currentGameObjects)
        if (go->_isReplicated)
            num++;

    return num;
}

void GameObjectManager::CreateWorldState(char* worldState)
{
    std::vector<GameObject*> replicatedObjects;
    for (const auto go : _currentGameObjects)
        if (go->_isReplicated)
            replicatedObjects.push_back(go);

	for (int i = 0; i < (int)replicatedObjects.size(); i++)
	{
        char goState[GAMEOBJECT_STATE_SIZE];
        CreateObjectState(replicatedObjects[i], goState);

        for (int j = 0; j < GAMEOBJECT_STATE_SIZE; j++)
            worldState[i * GAMEOBJECT_STATE_SIZE + j] = goState[j];
	}
}*/

/*void GameObjectManager::ReadWorldState(const char* state)
{
    printf("\nReading World State \n");
    
    int readIndex = BYTE_STREAM_OVERHEAD;
    const int num = (uint8)state[readIndex++];

    for (int i = 0; i < num; i++)
    {
        bool objectFound = false;
	    const char prefabID = state[readIndex++];
	    const char instanceID = state[readIndex++];
        for (const auto go : _currentGameObjects)
        {
            if(go->GetComponent<PlayerController>())
            {
	            const auto playerID = state[readIndex];
                if (go->GetComponent<PlayerController>()->GetPlayerID() == playerID)
                {
                    go->instanceID = instanceID;
                    readIndex++;
                }
            }

	        if(go->instanceID != instanceID) continue;

            printf("Setting Object \n");

            ReadGameObject(state, readIndex, go);
            objectFound = true;
            readIndex++;

            break;
        }

        if (prefabID > -1 && !objectFound)
        {
            printf("Creating Object \n");

	        const auto go = SpawnPrefab(PrefabManager::GetInstance().GetPrefabPath(prefabID));

            if (prefabID == PLAYER_PREFAB_ID)
                go->GetComponent<PlayerController>()->InitController(state[readIndex++]);

            ReadGameObject(state, readIndex, go);
            readIndex++;
        }
    }
}*/

/*void GameObjectManager::ReadObjectState(const char* state)
{
    int readIndex = BYTE_STREAM_OVERHEAD;

    const char prefabID = state[readIndex++];
    const char instanceID = state[readIndex++];

    for (const auto go : _currentGameObjects)
    {
        if (go->instanceID != (uint8)instanceID) continue;

        if (go->GetComponent<PlayerController>())
            readIndex++;

        ReadGameObject(state, readIndex, go);
        break;
    }
}

void GameObjectManager::ReadGameObject(const char* state, int& readIndex, GameObject* go)
{
    // Position
    int16 x = 0, y = 0;
    std::memcpy(&x, &state[readIndex], sizeof(int16));
    readIndex += sizeof(int16);

    std::memcpy(&y, &state[readIndex], sizeof(int16));
    readIndex += sizeof(int16);

    go->SetPosition(Vector2D(x, y));

    // Rotation
    int16 rot;
    std::memcpy(&rot, &state[readIndex], sizeof(int16));
    readIndex += sizeof(int16);

    go->transform.rotation = rot;
}*/

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
