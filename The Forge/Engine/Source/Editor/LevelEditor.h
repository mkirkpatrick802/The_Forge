#pragma once
#include <vector>

#include "json.hpp"
#include "Engine/UIWindow.h"

namespace Editor
{
    class LevelEditor final : public Engine::UIWindow
    {
    public:
        LevelEditor();
        void Render() override;
    
    private:
        std::vector<const char*> ConvertLevelDataToNameList(const std::vector<nlohmann::json>& levelData);
        void LevelSettings();

    private:
        char _levelNameBuffer[128] = "";

        std::vector<std::string> _levelNames;
        int _selectedItem = 0;
    };
}
