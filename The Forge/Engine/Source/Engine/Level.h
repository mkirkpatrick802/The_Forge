#pragma once
#include <string>
#include <vector>

#include "ByteStream.h"
#include "GameEngine.h"
#include "json.hpp"
#include "LevelManager.h"

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
        bool RemoveGameObject(const GameObject* go);

        void SaveLevel(const std::string& args = "");
        
        std::vector<GameObject*> GetAllGameObjects() const;
        std::string GetName() { return _name; }
        GameModeBase& GetGameMode() const { return *_gameMode; }

        
    private:
        void Write(NetCode::OutputByteStream& stream, bool isCompleteState = true) const;
        void Read(NetCode::InputByteStream& stream);
        
        Level(nlohmann::json data);
        ~Level();

        void Start() const;
        
    private:
        
        std::string _name;
        std::string _path;
        
        std::vector<std::unique_ptr<GameObject>> _gameObjects;
        std::unique_ptr<GameModeBase> _gameMode; // Only exists on the server
    };

    inline void Destroy(const GameObject* go)
    {
        LevelManager::GetCurrentLevel()->RemoveGameObject(go);
    }
}
