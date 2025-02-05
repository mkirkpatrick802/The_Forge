#pragma once
#include <vector>

namespace Engine
{
    class PlayerStart;
    class GameModeBase
    {
    public:
        GameModeBase() = default;
        ~GameModeBase() = default;

    private:

        std::vector<PlayerStart> _playerStarts;
        
    };
}
