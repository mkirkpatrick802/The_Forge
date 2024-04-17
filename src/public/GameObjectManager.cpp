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

GameObjectManager::GameObjectManager(Renderer* renderer, InputManager* inputManager) : _renderer(renderer), _inputManager(inputManager)
{
    RegisterComponentFns();
    LoadLevel();

    SubscribeToEvent(EventType::ET_SpawnPlayer);
    SubscribeToEvent(EventType::ET_DetailsChanged);
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

void GameObjectManager::CreateGameObjectFromJSON(const json &gameObject)
{

    GameObject* go = CreateGameObject();
    if(go == nullptr)
        assert(0 && "Failed to Create Game Object");

    std::string name = gameObject["Name"];
    go->_name = name;

    std::string position = gameObject["Position"];
    std::istringstream iss(position);
    iss >> go->_transform.position.x >> go->_transform.position.y;

    auto& components = gameObject["Components"];
    for (const auto& component : components)
        CreateComponentFromJSON(go, component);

    go->ObjectCreated();
}

void GameObjectManager::CreateComponentFromJSON(GameObject* go, const json &component)
{

    int componentID = component["ID"];
    auto it = componentCreationMap.find(componentID);
    if(it == componentCreationMap.end())
        assert(0 && "Could not find valid component function");

    (this->*componentCreationMap[componentID])(go, component);
}

GameObject* GameObjectManager::CreateGameObject()
{

    if(_currentGameObjects.size() >= MAX_GAMEOBJECTS)
        return nullptr;

    auto newObject = new GameObject();
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

        levelData["GameObjects"][i]["Name"] = _currentGameObjects[i]->_name;
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
    std::ifstream in(PLAYER_FILE);

    if (!in.is_open()) 
    {
        std::cerr << "Could not open file for reading!\n";
        assert(0);
    }

    json playerData;
    in >> playerData;
    in.close();

    playerData["Name"] = player->_name;
    playerData["Position"] = player->GetPositionString();

    std::ofstream out(PLAYER_FILE);
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
        SpawnPlayer();
        break;
	case EventType::ET_DetailsChanged:
        ToggleEditorMode(static_cast<DetailsChangedEvent*>(event)->currentDetails.editorSettings.editMode);
		break;
    }
}

void GameObjectManager::SpawnPlayer()
{
    std::ifstream file(PLAYER_FILE);

    if (!file.is_open()) {
        std::cerr << "Could not open file for reading!\n";
        assert(0);
    }

    json playerData;
    file >> playerData;
    file.close();

	CreateGameObjectFromJSON(playerData);
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