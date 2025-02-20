#pragma once
#include <memory>
#include <string>
#include <vector>

namespace Engine
{
    class GameObject;
    class PlayerStart;
    class GameModeBase
    {
    public:
        GameModeBase();
        ~GameModeBase() = default;

        void Start();
        GameObject* SpawnPlayer(uint64_t playerID) const;

    private:
        std::vector<PlayerStart*> _playerStarts;
        std::string _playerPrefab = "Assets/Player.prefab";
        
    };
}
