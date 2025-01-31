#pragma once
#include <string>
#include <vector>

#include "Components/ComponentUtils.h"
#include "json.hpp"

namespace Engine
{
    const String LEVEL_PATH = "Assets/Levels/";
    
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
        
        static Level* _currentLevel;
        std::unique_ptr<ComponentUtils> _componentUtils;
    };
}
