#pragma once
#include <string>
#include <vector>

#include "ByteStream.h"
#include "json.hpp"

namespace NetCode
{
    class NetworkManager;
}

namespace Engine
{
    class GameModeBase;
}

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
        friend class NetCode::NetworkManager;
        
    public:
        GameObject* SpawnNewGameObject(const std::string& filepath = "");
        bool RemoveGameObject(GameObject* go);

        void SaveLevel(const std::string& args = "");
        
        std::vector<GameObject*> GetAllGameObjects() { return _gameObjects; }
        std::string GetName() { return _name; }
        GameModeBase& GetGameMode() const { return *_gameMode; }

        
    private:
        void Write(NetCode::OutputByteStream& stream) const;
        void Read(NetCode::InputByteStream& stream);
        
        Level(nlohmann::json data);
        ~Level();

        void Start();
        
    private:
        
        std::string _name;
        std::string _path;
        
        std::vector<GameObject*> _gameObjects;
        std::unique_ptr<GameModeBase> _gameMode; // Only exists on the server
    };
}
