#pragma once
#include <string>
#include <vector>

#include "Data.h"
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
        
        std::vector<GameObject*> GetAllGameObjects() { return _gameObjects; }
        String GetName() { return _name; }

    private:
        
        Level(nlohmann::json data);
        ~Level();

        void SaveLevel(const String& args);
        
    private:
        String _name;
        String _path;
        
        std::vector<GameObject*> _gameObjects;
    };
}
