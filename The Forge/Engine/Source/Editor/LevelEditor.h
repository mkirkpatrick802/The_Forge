#pragma once
#include <vector>

#include "json.hpp"
#include "Engine/Rendering/CameraManager.h"
#include "Engine/Rendering/UIWindow.h"

namespace Engine
{
    class Level;
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

        void DeleteGameObjects(Engine::Level* currentLevel);
        void CreatePrefab(Engine::GameObject* go) const;
    private:

        std::string _defaultLevelFilePath;
        int _defaultLevelIndex = -1;
        bool _showRenameTextBox = false;

        static std::vector<nlohmann::json> levelData;
        static std::vector<std::string> filepaths;
        
        char _levelNameBuffer[128] = "";
        char _newGameObjectName[128] = "";

        std::vector<std::string> _levelNames;
        int _selectedLevel = 0;
        int _selectedGameObject = -1;

        Engine::GameObject* _gameObjectToDelete = nullptr;
    };
}
