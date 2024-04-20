//
// Created by mKirkpatrick on 1/30/2024.
//

#include "GameObjectManager.h"
#include "GameObject.h"
#include <iostream>
#include <iomanip>

#include <fstream>
#include <sstream>

#include "DetailsChangedEvent.h"
#include "Renderer.h"
#include "PrefabManager.h"
#include "SpawnPlayerEvent.h"
#include "SyncWorldEvent.h"
#include "UpdateObjectEvent.h"

std::vector<GameObject*> GameObjectManager::_currentGameObjects = std::vector<GameObject*>();

GameObjectManager::GameObjectManager(Renderer* renderer, InputManager* inputManager) : _renderer(renderer), _inputManager(inputManager)
{
    RegisterComponentFns();
    LoadLevel();

    SubscribeToEvent(EventType::ET_SpawnPlayer);
    SubscribeToEvent(EventType::ET_DetailsChanged);
    SubscribeToEvent(EventType::ET_SyncWorld);
    SubscribeToEvent(EventType::ET_UpdateObject);

    std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

void GameObjectManager::RegisterComponentFns()
{
    componentCreationMap[SPRITE_RENDERER] = &GameObjectManager::CreateSpriteRenderer;
    componentCreationMap[PLAYER_CONTROLLER] = &GameObjectManager::CreatePlayerController;
}

void GameObjectManager::LoadLevel()
{
    std::ifstream file(LEVEL_FILE);

    if (!file.is_open()) {
        std::cerr << "Could not open file for reading!\n";
        assert(0);
    }

    json levelData;
    file >> levelData;
    file.close();

    auto& gameObjects = levelData["GameObjects"];
    for (const auto& gameObject : gameObjects)
        CreateGameObjectFromJSON(gameObject);
}

GameObject* GameObjectManager::SpawnPrefab(const PrefabPath& path)
{
    std::ifstream file(path);

    if (!file.is_open()) {
        std::cerr << "Could not open file for reading!\n";
        assert(0);
    }

    json prefab;
    file >> prefab;
    file.close();

    return CreateGameObjectFromJSON(prefab);
}

GameObject* GameObjectManager::CreateGameObjectFromJSON(const json &gameObject)
{

    GameObject* go = CreateGameObject();
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
    go->_isReplicated = (bool)isReplicated;

    const std::string position = gameObject["Position"];
    std::istringstream iss(position);
    iss >> go->transform.position.x >> go->transform.position.y;

    auto& components = gameObject["Components"];
    for (const auto& component : components)
        CreateComponentFromJSON(go, component);

    go->ObjectCreated();
    return go;
}

void GameObjectManager::CreateComponentFromJSON(GameObject* go, const json &component)
{
	const int componentID = component["ID"];
	if(const auto it = componentCreationMap.find(componentID); it == componentCreationMap.end())
        assert(0 && "Could not find valid component function");

    (this->*componentCreationMap[componentID])(go, component);
}

GameObject* GameObjectManager::CreateGameObject()
{
    if(_currentGameObjects.size() >= MAX_GAMEOBJECTS)
        return nullptr;

    auto newObject = new GameObject();

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
    }
	while (!IsUnique);

    newObject->instanceID = newID;
    _currentGameObjects.push_back(newObject);

    return newObject;
}

bool GameObjectManager::AddComponent(GameObject *go, uint32 componentID)
{

    auto it = componentCreationMap.find(componentID);
    if(it == componentCreationMap.end())
        return false;

    (this->*componentCreationMap[componentID])(go, nullptr);
    return true;
}

void GameObjectManager::CreateSpriteRenderer(GameObject *go, const json& data = nullptr)
{
    if(_renderer != nullptr)
        _renderer->CreateSpriteRenderer(go, data);
}

void GameObjectManager::CreatePlayerController(GameObject *go, const json &data = nullptr)
{
    PlayerController* playerController = _playerControllerPool.New(go);
    playerController->SetInputManager(_inputManager);

    go->AddComponent(playerController);

    if(data != nullptr)
        playerController->LoadData(data);
}

void GameObjectManager::Update(float deltaTime)
{
    _playerControllerPool.Update(deltaTime);
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
    playerData["Is Replicated"] = (int)player->_isReplicated;
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
	    {
			const auto player = SpawnPrefab(PrefabManager::GetInstance().GetPrefabPath(PLAYER_PREFAB_ID));
			const int ID = static_cast<SpawnPlayerEvent*>(event)->playerID;
			player->GetComponent<PlayerController>()->InitController(ID);
	    }
        break;

	case EventType::ET_DetailsChanged:
        ToggleEditorMode(static_cast<DetailsChangedEvent*>(event)->currentDetails.editorSettings.editMode); // VS says this static cast is bad IDK why (Explore later)
		break;

	case EventType::ET_SyncWorld:
        ReadWorldState(static_cast<SyncWorldEvent*>(event)->worldState);
		break;
	case EventType::ET_UpdateObject:
        ReadObjectState(static_cast<UpdateObjectEvent*>(event)->objectState);
		break;
    }
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

int GameObjectManager::GetNumOfReplicatedObjects()
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
}

void GameObjectManager::CreateObjectState(const GameObject* object, char* state)
{
    int bufferPos = 0;

    // Prefab ID
    state[bufferPos++] = (char)object->_prefabID;

    // Instance ID
    state[bufferPos++] = (char)object->instanceID;

    // Player ID
    if (object->_prefabID == PLAYER_PREFAB_ID)
        state[bufferPos++] = (char)object->GetComponent<PlayerController>()->GetPlayerID();

    // Covert position from float to int16
    const int16 x = (int16)object->GetPosition().x;
    const int16 y = (int16)object->GetPosition().y;

    // Set x position
	std::memcpy(&state[bufferPos], &x, sizeof(int16));
    bufferPos += sizeof(int16);

    // Set y position
    std::memcpy(&state[bufferPos], &y, sizeof(int16));
}

void GameObjectManager::ReadWorldState(const char* state)
{
    printf("\nReading World State \n");

    int readIndex = 3;
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

            int16 x = 0, y = 0;
            std::memcpy(&x, &state[readIndex], sizeof(int16));
            readIndex += sizeof(int16);

            std::memcpy(&y, &state[readIndex], sizeof(int16));
            readIndex += sizeof(int16);

            go->SetPosition(Vector2D(x, y));
            readIndex += 3;
            objectFound = true;
            break;
        }

        if (prefabID > -1 && !objectFound)
        {
            printf("Creating Object \n");

	        const auto go = SpawnPrefab(PrefabManager::GetInstance().GetPrefabPath(prefabID));

            if (prefabID == PLAYER_PREFAB_ID)
                go->GetComponent<PlayerController>()->InitController(state[readIndex++]);

            int16 x = 0, y = 0;
            std::memcpy(&x, &state[readIndex], sizeof(int16));
            readIndex += sizeof(int16);

            std::memcpy(&y, &state[readIndex], sizeof(int16));
            readIndex += sizeof(int16);

            go->SetPosition(Vector2D(x, y));
            readIndex += 3;
        }
    }
}

void GameObjectManager::ReadObjectState(const char* state)
{
    int readIndex = 3;

    const char prefabID = state[readIndex++];
    const char instanceID = state[readIndex++];

    for (const auto go : _currentGameObjects)
    {
        if (go->instanceID != (uint8)instanceID) continue;

        if (go->GetComponent<PlayerController>())
            readIndex++;

        int16 x = 0, y = 0;
        std::memcpy(&x, &state[readIndex], sizeof(int16));
        readIndex += sizeof(int16);

        std::memcpy(&y, &state[readIndex], sizeof(int16));
        readIndex += sizeof(int16);

        go->SetPosition(Vector2D(x, y));
        break;
    }
}

void GameObjectManager::CleanUp()
{
    const uint32 size = _currentGameObjects.size();
    for (int i = 0; i < size; ++i)
    {
        if(PlayerController* controller = _currentGameObjects[i]->GetComponent<PlayerController>())
            _playerControllerPool.Delete(controller);

        _renderer->CleanUpSpriteRenderer(_currentGameObjects[i]);

        delete _currentGameObjects[i];
    }
    _currentGameObjects.clear();
}
