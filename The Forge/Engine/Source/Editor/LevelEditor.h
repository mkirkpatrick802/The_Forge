#pragma once
#include <vector>

#include "json.hpp"
#include "Engine/Data.h"
#include "Engine/UIWindow.h"

namespace Engine
{
    class GameObject;
}

namespace Editor
{
    class LevelEditor final : public Engine::UIWindow
    {
    public:
        LevelEditor();
        ~LevelEditor() override;
        void Render() override;
    
    private:
        std::vector<const char*> ConvertLevelDataToNameList(const std::vector<nlohmann::json>& levelData);
        void LevelSettings();

    private:

        String _defaultLevelFilePath;
        int _defaultLevelIndex = -1;
        
        static std::vector<nlohmann::json> levelData;
        static std::vector<String> filepaths;
        
        char _levelNameBuffer[128] = "";

        std::vector<std::string> _levelNames;
        int _selectedLevel = 0;
        int _selectedGameObject = -1;
    };
}
