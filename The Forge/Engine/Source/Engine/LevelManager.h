#pragma once
#include <string>

namespace Engine
{
    class Level;
    class LevelManager
    {
        friend class GameEngine;
        
    public:
        static bool LoadLevel(const std::string& filepath);
        
    public:
        static Level* GetCurrentLevel() { return _currentLevel; }

    private:
        LevelManager(const std::string& filepath);
        
    private:
        static Level* _currentLevel;
    };
}
