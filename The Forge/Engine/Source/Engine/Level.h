#pragma once
#include <string>
#include <vector>

#include "json.hpp"

namespace Engine
{
    class GameObject;
    class Level
    {
        friend class LevelManager;
        
    public:
        
        bool SpawnNewGameObject();
        std::vector<GameObject*> GetAllGameObjects() { return _gameObjects; }
        std::string GetName() { return _name; }

    private:
        
        Level(nlohmann::json data);
        ~Level();
        
    private:
        std::string _name;
        
        std::vector<GameObject*> _gameObjects;
        std::string _path;
    };
}
