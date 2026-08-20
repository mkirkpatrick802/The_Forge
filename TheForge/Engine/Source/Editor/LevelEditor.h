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

        // The single way to change what is selected.
        //
        // Selection lives in two places -- DetailsEditor holds the GameObject* that the
        // inspector draws, and the hierarchy holds an *index* for its highlight -- and
        // they used to be set independently, so picking in the viewport would have left
        // the hierarchy pointing at whatever was selected before. Passing nullptr clears
        // both.
        static void SelectGameObject(Engine::GameObject* go);
    
    private:
        
        std::vector<const char*> ConvertLevelDataToNameList(const std::vector<nlohmann::json>& levelData);

        // Replaces the level pickers while a prefab is open, since none of them apply.
        // Returns false once it has closed the prefab, which invalidates everything the
        // rest of the window was about to draw.
        static bool PrefabBar();
        void LevelSettings();
        void Hierarchy();
        void RenderGameObjectHierarchy(int index);
        
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
        static int _selectedGameObject;

        Engine::GameObject* _gameObjectToDelete = nullptr;
    };
}
