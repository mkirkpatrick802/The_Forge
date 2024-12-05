#include "Level.h"

#include <fstream>

#include "GameObject.h"
#include "JsonKeywords.h"
#include "System.h"

Engine::Level::Level(nlohmann::json data)
{
    _name = data[JsonKeywords::LEVEL_NAME];

    //Load Game Objects From Json Data
}

Engine::Level::~Level()
{
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
        System::DisplayMessageBox("Error", "Could not open file: " + _path);
        return false;
    }

    //Update Json Data
    nlohmann::json data;
    readFile >> data;

    if (!data.contains(JsonKeywords::GAMEOBJECT_ARRAY))
        data[JsonKeywords::GAMEOBJECT_ARRAY] = nlohmann::json::array();
    
    data[JsonKeywords::GAMEOBJECT_ARRAY].push_back(go->_data);

    //Open Level File To Write
    std::ofstream writeFile(_path);
    if (!readFile.is_open())
    {
        System::DisplayMessageBox("Error", "Could not open file: " + _path);
        return false;
    }

    writeFile << data.dump(4);
    
    return true;
}
