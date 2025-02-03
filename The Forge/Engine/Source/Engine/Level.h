#pragma once
#include <string>
#include <vector>
#include "json.hpp"

namespace Editor
{
    class DetailsEditor;
}

namespace Engine
{
    class Level;
    class GameObject;
    
    class Level
    {
        friend class LevelManager;
        friend class Editor::DetailsEditor;
        
    public:
        
        bool SpawnNewGameObject();
        bool RemoveGameObject(GameObject* go);
        
        std::vector<GameObject*> GetAllGameObjects() { return _gameObjects; }
        std::string GetName() { return _name; }

    private:
        
        Level(nlohmann::json data);
        ~Level();

        void SaveLevel(const std::string& args);
        
    private:
        std::string _name;
        std::string _path;
        
        std::vector<GameObject*> _gameObjects;
    };
}
