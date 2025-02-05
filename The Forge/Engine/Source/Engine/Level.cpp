#include "Level.h"

#include <fstream>

#include "CommandRegistry.h"
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

bool Engine::Level::SpawnNewGameObject()
{
    // Create Game Object
    const auto go = DEBUG_NEW GameObject();
    _gameObjects.push_back(go);

    //Open Level File To Read
    std::ifstream readFile(_path);
    if (!readFile.is_open())
    {
        System::GetInstance().DisplayMessageBox("Error", "Could not open file: " + _path);
        return false;
    }

    //Update Json Data
    nlohmann::json data;
    readFile >> data;

    if (!data.contains(JsonKeywords::GAMEOBJECT_ARRAY))
        data[JsonKeywords::GAMEOBJECT_ARRAY] = nlohmann::json::array();
    
    data[JsonKeywords::GAMEOBJECT_ARRAY].push_back(go->_defaultData);

    //Open Level File To Write
    std::ofstream writeFile(_path);
    if (!readFile.is_open())
    {
        System::GetInstance().DisplayMessageBox("Error", "Could not open file: " + _path);
        return false;
    }

    writeFile << data.dump(4);
    
    return true;
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
        System::GetInstance().DisplayMessageBox("Message", "Current Level Saved!!");
        outputFile.close();
    }
}
