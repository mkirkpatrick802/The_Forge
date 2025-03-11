#include "LevelEditor.h"

#include <fstream>
#include <iostream>

#include "DetailsEditor.h"
#include "Engine/Rendering/ImGuiHelper.h"
#include "Engine/EngineManager.h"
#include "Engine/GameObject.h"
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
    static int selectedGameMode = 0;
    if (ImGui::Combo("##GameMode", &selectedGameMode, gamemodes.data(), static_cast<int>(gamemodes.size())))
    {
        
    }
    ImGui::PopItemWidth();

    ImGui::Spacing();
    Engine::ImGuiHelper::InputVector2("Level Size: ", currentLevel->_size, 100);
    
    // Game Objects
    ImGui::Separator();
    
    if (Engine::ImGuiHelper::CenteredButtonWithPadding("Create New Game Object", 5))
    {
        currentLevel->SpawnNewGameObject();
    }

    Hierarchy();
}

void Editor::LevelEditor::Hierarchy()
{
    const auto currentLevel = Engine::LevelManager::GetCurrentLevel();
    if (currentLevel == nullptr) return;

    ImGui::Text("Hierarchy");
    ImGui::PushItemWidth(50);
    ImGui::Separator();

    const auto levelObjects = currentLevel->GetAllGameObjects();
    for (int i = 0; i < levelObjects.size(); ++i)
    {
        // Only render root-level objects (objects with no parent)
        if (levelObjects[i]->GetParent() == nullptr)
        {
            RenderGameObjectHierarchy(i);  // Recursively render the hierarchy for the root objects
        }
    }

    ImGui::PopItemWidth();

    // Handle drag-and-drop outside any valid hierarchy item (empty space area)
    ImGui::Dummy(ImGui::GetWindowSize());
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GAME_OBJECT"))
        {
            int droppedObject = *(int*)payload->Data;

            // If the drop is happening in the empty space, we remove the child from its parent
            if (levelObjects[droppedObject] && levelObjects[droppedObject]->GetParent())
            {
                // Remove the dropped object from its current parent
                levelObjects[droppedObject]->GetParent()->RemoveChild(levelObjects[droppedObject]);
            }
        }
        ImGui::EndDragDropTarget();
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

void Editor::LevelEditor::RenderGameObjectHierarchy(const int index)
{
    const auto currentLevel = Engine::LevelManager::GetCurrentLevel();
    if (currentLevel == nullptr) return;

    const auto levelObjects = currentLevel->GetAllGameObjects();
    if (index < 0 || index >= (int)levelObjects.size()) return;
    
    Engine::GameObject* parentObject = levelObjects[index];

    int flags = ImGuiTreeNodeFlags_DefaultOpen;
    if(index == _selectedGameObject)
        flags |= ImGuiTreeNodeFlags_Selected;
    
    ImGui::PushID(index);
    const bool open = ImGui::TreeNodeEx(parentObject->GetName().c_str(), flags);
    
    // Handle right-click context menu
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
    {
        ImGui::OpenPopup("Context Menu");
    }

    if (ImGui::BeginPopup("Context Menu"))
    {
        if (ImGui::MenuItem("Create Prefab"))
        {
            CreatePrefab(levelObjects[index]);
        }

        if (ImGui::MenuItem("Rename"))
        {
            _showRenameTextBox = true;
        }

        if (ImGui::MenuItem("Delete"))
        {
            _gameObjectToDelete = levelObjects[index];
        }

        ImGui::EndPopup();
    }

    ImGui::PopID();

    if (ImGui::IsItemClicked())
    {
        _selectedGameObject = index;  // Set the selected object
        DetailsEditor::SetSelectedGameObject(parentObject);  // Update the editor with the selected game object
    }

    // Handle drag source (dragging the object itself)
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
    {
        // Set the drag payload (pass the index of the object)
        ImGui::SetDragDropPayload("GAME_OBJECT", &index, sizeof(index));
        ImGui::EndDragDropSource();  // End the drag source
    }

    // Handle drop target (where the object is being dragged onto)
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GAME_OBJECT"))
        {
            // Get the dropped object index from the payload
            const int droppedIndex = *(int*)payload->Data;

            // Ensure we're not re-parenting the object to itself
            if (droppedIndex != index) 
            {
                Engine::GameObject* droppedObject = levelObjects[droppedIndex];
                if(droppedObject->GetParent())
                    droppedObject->GetParent()->RemoveChild(droppedObject);
                
                parentObject->AddChild(droppedObject);  // Add the dropped object as a child
            }
        }
        ImGui::EndDragDropTarget();
    }

    if(open)
    {
        // Render the children of this game object recursively
        for (const auto children = parentObject->GetChildren(); const Engine::GameObject* child : children)
        {
            for (int i = 0; i < levelObjects.size(); i++)
            {
                if (levelObjects[i] == child)
                    RenderGameObjectHierarchy(i);  // Recursively render each child
            }
        }

        ImGui::TreePop();  // End the current tree node
    }
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
