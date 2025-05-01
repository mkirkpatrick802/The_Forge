#pragma once
#include <string>
#include <vector>

#include "ByteStream.h"
#include "json.hpp"

namespace Engine
{
    const std::string LEVEL_PATH = "Assets/Levels/";
    
    class Level;
    class LevelManager
    {
        friend class GameEngine;
        
    public:
        
        static bool LoadLevel(const std::string& filepath);
        static bool LoadLevel(NetCode::InputByteStream& stream);
        static bool CreateLevel(const std::string& levelName);

        LevelManager(const std::string& filepath);
        void StartCurrentLevel() const;
        
    public:
        
        static Level* GetCurrentLevel() { return _currentLevel; }

        //Returns the list of file paths, and gives out the list of json data
        static std::vector<std::string> GetAllLevels(std::vector<nlohmann::json>& levelData);

    private:
        
        void CleanUp() const;
        
    private:
        
        static Level* _currentLevel;
        
    };
}
