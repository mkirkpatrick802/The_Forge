#include "LevelEditor.h"

#include <iostream>

#include "ImGuiHelper.h"
#include "Engine/GameObject.h"
#include "Engine/JsonKeywords.h"
#include "Engine/Level.h"
#include "Engine/LevelManager.h"

Editor::LevelEditor::LevelEditor()
{
    
}

void Editor::LevelEditor::Render()
{
    static auto once = true;
    if (once)
    {
        once = false;
        
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags &= ImGuiConfigFlags_ViewportsEnable;
        static ImGuiDockNodeFlags flags = ImGuiDockNodeFlags_PassthruCentralNode;
    }

    ImGui::Begin("Level Editor");
    if (ImGui::BeginTable("Level Editor", 2, ImGuiTableFlags_SizingStretchSame |
        ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersOuter ))
    {
        // Create New Level Side
        ImGui::TableNextColumn();
        ImGui::Text("Create New Level:");

        // Level Name
        ImGui::InputText("Name", _levelNameBuffer, IM_ARRAYSIZE(_levelNameBuffer));

        // Create Level
        if (ImGui::Button("Create New Level"))
        {
            Engine::LevelManager::CreateLevel(_levelNameBuffer);
        }

        // Load New Level Side
        ImGui::TableNextColumn();
        ImGui::Text("Load Level:");

        std::vector<nlohmann::json> levelData;
        auto filepaths = Engine::LevelManager::GetAllLevels(levelData);

        std::vector<const char*> levels;
        if (!levelData.empty())
            levels = ConvertLevelDataToNameList(levelData);
        
        if (ImGui::Combo("", &_selectedItem, levels.data(), static_cast<int>(levels.size())))
        {
            Engine::LevelManager::LoadLevel(filepaths[_selectedItem]);
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
    if (currentLevel== nullptr) return;

    const std::string nameText = "Level Name: " + currentLevel->GetName();
    ImGui::Text(nameText.c_str());
    
    if (ImGuiHelper::CenteredButtonWithPadding("Create New Game Object", 5))
    {
        currentLevel->SpawnNewGameObject();
    }

    ImGui::Separator();
    ImGui::Text("Hierarchy");

    static int selectedIndex = -1;
    const auto levelObjects = currentLevel->GetAllGameObjects();
    for (int i = 0; i < levelObjects.size(); ++i)
    {
        // Highlight the selected item
        bool isSelected = (selectedIndex == i);
        ImGui::PushID(i);
        if (ImGui::Selectable(levelObjects[i]->GetName().c_str(), isSelected))
        {
            selectedIndex = i; // Update the selected item
        }
        ImGui::PopID();
    }
}