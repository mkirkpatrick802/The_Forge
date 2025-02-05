#pragma once
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

    private:

        std::vector<PlayerStart*> _playerStarts;
        
    };
}
