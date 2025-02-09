#include "Level.h"
#include <fstream>

#include "CommandRegistry.h"
#include "GameModeBase.h"
#include "Components/Component.h"
#include "GameObject.h"
#include "JsonKeywords.h"
#include "System.h"

Engine::Level::Level(nlohmann::json data)
{
    CommandRegistry::RegisterCommand("/save", [this](const std::string& args){ SaveLevel(args); });
    
    _name = data[JsonKeywords::LEVEL_NAME];

    //Load Game Objects From Json Data
    if (!data.contains(JsonKeywords::GAMEOBJECT_ARRAY)) return;

    for (const auto& go_data : data[JsonKeywords::GAMEOBJECT_ARRAY])
    {
        const auto go = DEBUG_NEW GameObject(go_data);
        _gameObjects.push_back(go);
    }

    _gameMode = std::make_unique<GameModeBase>();
}

Engine::Level::~Level()
{
    CommandRegistry::UnregisterCommand("/save");
    
    // Clean Up all the game objects
    for (auto go : _gameObjects)
    {
        delete go;
        go = nullptr;
    }

    _gameObjects.clear();
}

void Engine::Level::Start()
{
    _gameMode->Start();
}

Engine::GameObject* Engine::Level::SpawnNewGameObject(const std::string& filepath)
{
    // Create Game Object
    const auto go = DEBUG_NEW GameObject();
    if (!filepath.empty())
    {
        std::ifstream file(filepath);
        if (!file.is_open())
        {
            std::cerr << "Failed to open file " << filepath << '\n';
            return nullptr;
        }
        
        json j;
        file >> j; // Parse JSON from the file stream
        go->Deserialize(j);
    }
    
    _gameObjects.push_back(go);

    return go;
}

bool Engine::Level::RemoveGameObject(GameObject* go)
{
    std::erase(_gameObjects, go);
    delete go;
    return true;
}

void Engine::Level::SaveLevel(const std::string& args)
{
    nlohmann::json data;
    
    // Update level data
    data[JsonKeywords::LEVEL_NAME] = _name;

    // Update game objects
    data[JsonKeywords::GAMEOBJECT_ARRAY] = json::array();
    for (const auto go : _gameObjects)
        data[JsonKeywords::GAMEOBJECT_ARRAY].push_back(go->Serialize());

    // Write new data
    if (std::ofstream outputFile(_path); outputFile.is_open())
    {
        outputFile << data.dump(4);
        outputFile.close();
    }

    std::cout << "Level Saved Successfully!" << '\n';
}
