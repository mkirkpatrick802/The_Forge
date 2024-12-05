#pragma once
#include <string>
#include <vector>

#include "json.hpp"

namespace Engine
{
    class Level;
    class LevelManager
    {
        friend class GameEngine;
        
    public:
        static bool LoadLevel(const std::string& filepath);
        static bool CreateLevel(const std::string& levelName);
        
    public:
        static Level* GetCurrentLevel() { return _currentLevel; }

        //Returns the list of file paths, and gives out the list of json data
        static std::vector<std::string> GetAllLevels(std::vector<nlohmann::json>& levelData);

    private:
        LevelManager(const std::string& filepath);
        void CleanUp();
        
    private:
        const static std::string LEVEL_PATH;
        
        static Level* _currentLevel;
    };
}
