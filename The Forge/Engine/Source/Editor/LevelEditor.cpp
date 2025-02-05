#include "LevelEditor.h"

#include <fstream>
#include <iostream>

#include "DetailsEditor.h"
#include "ImGuiHelper.h"
#include "Engine/EngineManager.h"
#include "Engine/GameObject.h"
#include "Engine/JsonKeywords.h"
#include "Engine/Level.h"
#include "Engine/LevelManager.h"

std::vector<nlohmann::json> Editor::LevelEditor::levelData;
std::vector<std::string> Editor::LevelEditor::filepaths;

Editor::LevelEditor::LevelEditor()
{
    if (const auto defaultData = Engine::GetEngineManager().GetConfigData(Engine::DEFAULTS_FILE, JsonKeywords::Config::DEFAULT_LEVEL); defaultData.is_string())
        _defaultLevelFilePath = defaultData;
}

Editor::LevelEditor::~LevelEditor()
{
    levelData.clear();
    levelData.shrink_to_fit();

    filepaths.clear();
    filepaths.shrink_to_fit();
}

void Editor::LevelEditor::Render()
{
    ImGui::Begin("Level Editor");
    if (ImGui::BeginTable("Level Editor", 2))
    {
        // Create New Level Side
        ImGui::TableNextColumn();
        ImGui::Text("Create New Level:");

        // Level Name
        ImGui::PushItemWidth(100);
        ImGui::InputText("Name", _levelNameBuffer, IM_ARRAYSIZE(_levelNameBuffer));
        
        if (levelData.empty() || filepaths.empty())
            filepaths = Engine::LevelManager::GetAllLevels(levelData);
        
        // Create Level
        if (ImGui::Button("Create New Level"))
        {
            Engine::LevelManager::CreateLevel(_levelNameBuffer);
            
            levelData.clear();
            filepaths = Engine::LevelManager::GetAllLevels(levelData);
        }

        // Load New Level Side
        ImGui::TableNextColumn();

        std::vector<const char*> levels;
        if (!levelData.empty())
            levels = ConvertLevelDataToNameList(levelData);

        if (_defaultLevelIndex == -1)
        {
            for (int i = 0; i < levels.size(); i++)
            {
                if (levels[i] == _defaultLevelFilePath)
                {
                    _defaultLevelIndex = i;
                    _selectedLevel = i;

                    //TODO: Move this to Level Manager
                    //Engine::LevelManager::LoadLevel(filepaths[i]);
                }
            }   
        }
        
        ImGui::PushItemWidth(100);
        if (ImGui::Combo("Current Level", &_selectedLevel, levels.data(), static_cast<int>(levels.size())))
        {
            DetailsEditor::ClearSelectedGameObject();
            Engine::LevelManager::LoadLevel(filepaths[_selectedLevel]);
            _selectedGameObject = -1;
        }
        
        ImGui::Spacing();
        
        ImGui::PushItemWidth(100);
        if (ImGui::Combo("Default Level", &_defaultLevelIndex, levels.data(), static_cast<int>(levels.size())))
        {
            const std::string newDefaultLevel(levels[_defaultLevelIndex]);
            Engine::GetEngineManager().UpdateConfigFile(Engine::DEFAULTS_FILE, JsonKeywords::Config::DEFAULT_LEVEL, newDefaultLevel);
        }
        
        ImGui::EndTable();
    }

    LevelSettings();
    
    ImGui::End();
}

std::vector<const char*> Editor::LevelEditor::ConvertLevelDataToNameList(const std::vector<nlohmann::json>& levelData)
{
    // Extract level names from the JSON data
    _levelNames.clear();
    for (const auto& data : levelData)
    {
        try
        {
            // Assuming the JSON contains a "name" field for each level
            if (data.contains(JsonKeywords::LEVEL_NAME) && data[JsonKeywords::LEVEL_NAME].is_string())
            {
                _levelNames.push_back(data[JsonKeywords::LEVEL_NAME].get<std::string>());
            }
        }
        catch (const std::exception& e)
        {
            // Log and skip invalid JSON entries
            std::cerr << "Error processing JSON data: " << e.what() << '\n';
        }
    }

    std::vector<const char*> cStrings;
    cStrings.reserve(_levelNames.size());
    for (const auto& name : _levelNames)
    {
        cStrings.push_back(name.c_str());
    }

    return cStrings;
}

void Editor::LevelEditor::LevelSettings()
{
    const auto currentLevel = Engine::LevelManager::GetCurrentLevel();
    if (currentLevel == nullptr) return;

    // Game Mode
    ImGui::Separator();
    ImGui::Text("Level Settings");
    ImGui::Text("Level Game Mode:");
    ImGui::SameLine();

    // TODO: Make game mode objects
    ImGui::PushItemWidth(100);
    std::vector gamemodes = {"Main Menu", "Game"};
    int selectedGameMode = 0;
    if (ImGui::Combo(" ", &selectedGameMode, gamemodes.data(), static_cast<int>(gamemodes.size())))
    {
        
    }
    
    // Game Objects
    ImGui::Separator();
    
    if (ImGuiHelper::CenteredButtonWithPadding("Create New Game Object", 5))
    {
        currentLevel->SpawnNewGameObject();
    }
    
    ImGui::Text("Hierarchy");
    ImGui::PushItemWidth(50);
    ImGui::Separator();
    
    auto levelObjects = currentLevel->GetAllGameObjects();
    for (int i = 0; i < levelObjects.size(); ++i)
    {
        // Highlight the selected item
        bool isSelected = (_selectedGameObject == i);
        ImGui::PushID(i);
        if (ImGui::Selectable(levelObjects[i]->GetName().c_str(), isSelected))
        {
            _selectedGameObject = i; // Update the selected item
            DetailsEditor::SetSelectedGameObject(levelObjects[i]);
        }

        if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
        {
            // Open the context menu
            ImGui::OpenPopup("Context Menu");
        }

        if (ImGui::BeginPopup("Context Menu"))
        {
            if (ImGui::MenuItem("Create Prefab"))
            {
                CreatePrefab(levelObjects[_selectedGameObject]);
            }
            
            if (ImGui::MenuItem("Rename"))
            {
                _showRenameTextBox = true;
            }
            
            if (ImGui::MenuItem("Delete"))
            {
                _gameObjectToDelete = levelObjects[i];
            }
            
            ImGui::EndPopup();
        }
        
        ImGui::PopID();
    }

    // Conditionally show a text box for renaming
    if (_showRenameTextBox && _selectedGameObject > -1)
    {
        ImGui::OpenPopup("Rename Game Object");
        if (ImGui::BeginPopupModal("Rename Game Object", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Enter a new name:");
            ImGui::InputText("New Name", _newGameObjectName, sizeof(_newGameObjectName));

            if (ImGui::Button("OK"))
            {
                levelObjects[_selectedGameObject]->SetName(_newGameObjectName);
                _newGameObjectName[0] = '\0';
                _showRenameTextBox = false; // Close the modal after input
            }

            if (ImGui::Button("Cancel"))
            {
                _newGameObjectName[0] = '\0';
                _showRenameTextBox = false; // Close the modal without saving
            }

            ImGui::EndPopup();
        }
    }

    DeleteGameObjects(currentLevel);
}

void Editor::LevelEditor::DeleteGameObjects(Engine::Level* currentLevel)
{
    if (_gameObjectToDelete == nullptr) return;
    currentLevel->RemoveGameObject(_gameObjectToDelete);
    
    _gameObjectToDelete = nullptr;

    DetailsEditor::ClearSelectedGameObject();
}

void Editor::LevelEditor::CreatePrefab(Engine::GameObject* go) const
{
    if (go == nullptr) return;

    const json data = go->Serialize();
    if (std::ofstream file("Assets/" + go->GetName() + ".prefab"); file.is_open()) {
        file << data.dump(4); // Pretty print with 4 spaces
        file.close();
    }
}
