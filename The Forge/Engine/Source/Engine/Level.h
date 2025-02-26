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

namespace Editor
{
    class DetailsEditor;
}

namespace Engine
{
    class GameModeBase;
    class Level;
    class GameObject;
    
    class Level
    {
        friend class LevelManager;
        friend class Editor::DetailsEditor;
        friend class NetCode::NetworkManager;
        
    public:
        GameObject* SpawnNewGameObject(const std::string& filepath = "");
        bool RemoveGameObject(GameObject* go, bool replicate = true);

        void SaveLevel(const std::string& args = "");
        
        std::vector<GameObject*> GetAllGameObjects() const;
        std::string GetName() { return _name; }
        GameModeBase& GetGameMode() const { return *_gameMode; }
        
    private:
        void Write(NetCode::OutputByteStream& stream, bool isCompleteState = true);
        void Read(NetCode::InputByteStream& stream);
        
        Level(nlohmann::json data);
        Level(NetCode::InputByteStream& stream);
        ~Level();

        void Start() const;
        
    private:
        
        std::string _name;
        std::string _path;
        
        std::vector<std::unique_ptr<GameObject>> _gameObjects;
        std::vector<GameObject*> _destroyedObjects;
        std::unique_ptr<GameModeBase> _gameMode; // Only exists on the server
    };

    inline void Destroy(GameObject* go)
    {
        LevelManager::GetCurrentLevel()->RemoveGameObject(go);
    }
}
