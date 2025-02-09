#pragma once
#include <string>
#include <vector>

namespace Engine
{
    class PlayerStart;
    class GameModeBase
    {
    public:
        
        GameModeBase();
        ~GameModeBase() = default;

        void Start();
        void SpawnPlayer();

    private:

        std::vector<PlayerStart*> _playerStarts;
        std::string _playerPrefab = "Assets/Player.prefab";
        
    };
}
