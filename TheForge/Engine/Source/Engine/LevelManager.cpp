#include "LevelManager.h"

#include <fstream>
#include <iostream>

#include "EngineManager.h"
#include "LoadProgress.h"
#include "json.hpp"
#include "JsonKeywords.h"
#include "Level.h"
#include "System.h"
#include "Components/ComponentRegistry.h"
#include "Editor/DetailsEditor.h"
#include "Editor/EditorCamera.h"
#include "Editor/LevelEditor.h"
#include "Rendering/CameraManager.h"

Engine::Level* Engine::LevelManager::_currentLevel = nullptr;
std::string Engine::LevelManager::_levelBeforePrefab;

Engine::LevelManager::LevelManager(const std::string& filepath)
{
    if (const std::ifstream file(filepath); file.is_open())
        LoadLevel(filepath);

    if (!GetEngineManager().IsEditorEnabled())
        StartCurrentLevel();
}

void Engine::LevelManager::StartCurrentLevel() const
{
    // Play from a prefab means play the level it was opened from, not the prefab. The
    // prefab is written back on the way out, so pressing Play does not lose the edit.
    ClosePrefab();

    if (_currentLevel == nullptr)
    {
        DEBUG_LOG("Level: nothing to start -- no level is open")
        return;
    }

    // Deliberately does NOT save.
    //
    // This used to write the level to disk on the way into play, which sounds harmless
    // and is not: by the second play the "level" being saved already contains everything
    // the first play spawned -- the pawn, the asteroids, the ships. Those get baked into
    // the authored file and reloaded as real level content next time, so the world grows
    // every time you press Play and stops resembling what you built. Measured across
    // four play/editor cycles: 23 objects and 1 pawn became 39 objects and 2 pawns.
    //
    // Saving edits is what /save and the editor's own save are for.
    GetEngineManager().ToggleEditor("0");

    // Need to make a copy of the file path because it will get deleted in the LoadLevel function
    const std::string path = GetCurrentLevel()->_path;
    LoadLevel(path);
    
    DEBUG_LOG("Level: starting '%s'", path.c_str())

    // Start() first: the Camera component lives on the player prefab, and the player is
    // spawned from here by the game mode. Choosing a camera before that ran meant
    // choosing from an empty pool on the first play of a session.
    _currentLevel->Start();

    if (const auto cameras = GetComponentManager().GetPool<Camera>()->GetActive(); !cameras.empty())
        GetCameraManager().SetActiveCamera(cameras[0]);
}

void Engine::LevelManager::ReleaseCurrentLevel()
{
    // Clears the details panel and the hierarchy's highlight together -- both index the
    // level that is about to go.
    Editor::LevelEditor::SelectGameObject(nullptr);

    delete _currentLevel;
    _currentLevel = nullptr;
}

void Engine::LevelManager::CleanUp() const
{
    ReleaseCurrentLevel();
}

bool Engine::LevelManager::IsEditingPrefab()
{
    return _currentLevel != nullptr && _currentLevel->IsPrefab();
}

bool Engine::LevelManager::OpenPrefab(const std::string& filepath)
{
    std::ifstream prefab(filepath);
    if (!prefab.is_open())
    {
        System::GetInstance().DisplayMessageBox("Error", "Unable to open prefab for reading: " + filepath);
        return false;
    }

    nlohmann::json data;
    try
    {
        prefab >> data;
    }
    catch (const std::exception& e)
    {
        System::GetInstance().DisplayMessageBox("Error", "Prefab '" + filepath + "' is not valid JSON: " + e.what());
        return false;
    }
    prefab.close();

    // Opening a prefab from inside another one keeps the way back to the *level*. It
    // also has to write the one being left, which the level swap below would otherwise
    // discard silently.
    if (_currentLevel != nullptr)
    {
        if (_currentLevel->IsPrefab())
            _currentLevel->SaveLevel();
        else
            _levelBeforePrefab = _currentLevel->_path;
    }

    ReleaseCurrentLevel();

    // Current before Load, not after: GameObject::Deserialize spawns children into
    // whatever level is current, so the prefab's children have to find this one.
    _currentLevel = new Level();
    _currentLevel->_path = filepath;
    _currentLevel->LoadPrefab(data);

    // Opened, so: selected and looked at. A prefab is what the editor is now for, and
    // it stands wherever its own transform puts it -- which is not where the camera was
    // left in the level it replaced.
    if (GameObject* root = _currentLevel->FindRootObject())
    {
        Editor::EditorCamera::GetInstance()->FocusOn(root->GetWorldPosition());
        Editor::LevelEditor::SelectGameObject(root);
    }

    DEBUG_LOG("Prefab: editing '%s'", filepath.c_str())
    return true;
}

void Engine::LevelManager::ClosePrefab()
{
    if (!IsEditingPrefab()) return;

    _currentLevel->SaveLevel();

    const std::string returnTo = _levelBeforePrefab;
    _levelBeforePrefab.clear();

    if (returnTo.empty())
    {
        ReleaseCurrentLevel();
        return;
    }

    LoadLevel(returnTo);
}

bool Engine::LevelManager::CreateLevel(const std::string& levelName)
{
    const std::string filepath = LEVEL_PATH + levelName + ".json";

    if (std::filesystem::exists(filepath))
    {
        System::GetInstance().DisplayMessageBox("Error", "File already exists!");
        return false;
    }
    
    std::ofstream file(filepath);
    if (!file.is_open())
    {
        std::cerr << "Error: Unable to open file for writing: " << filepath << '\n';
        return false;
    }

    //Write Level Json
    nlohmann::json data;
    data[JsonKeywords::LEVEL_NAME] = levelName;
    file << data.dump(4);
    std::cout << '\n' << levelName << " Level Created!\n";
    
    file.close();
    return true;
}

std::vector<std::string> Engine::LevelManager::GetAllLevels(std::vector<nlohmann::json>& levelData)
{
    std::vector<std::string> levelFilePaths;

    // Ensure the path exists and is a directory
    if (!std::filesystem::exists(LEVEL_PATH) || !std::filesystem::is_directory(LEVEL_PATH))
    {
        throw std::runtime_error("LEVEL_PATH is invalid or does not exist.");
    }

    // Iterate through all files in the directory
    for (const auto& entry : std::filesystem::directory_iterator(LEVEL_PATH))
    {
        // Check if the entry is a regular file
        if (entry.is_regular_file())
        {
            const auto& filePath = entry.path();
            if (filePath.extension() == ".json")
            {
                // Attempt to read and parse the JSON data
                try
                {
                    std::ifstream fileStream(filePath);
                    if (fileStream.is_open())
                    {
                        nlohmann::json jsonData;
                        fileStream >> jsonData;
                        levelData.push_back(jsonData);
                    }
                }
                catch (const std::exception& e)
                {
                    // Log error and continue
                    std::cerr << "Error reading or parsing file " << filePath << ": " << e.what() << '\n';
                    std::cerr << "Level file corrupted!!" << '\n';
                    
                    continue;
                }

                levelFilePaths.push_back(filePath.string());
            }
        }
    }

    return levelFilePaths;
}

bool Engine::LevelManager::LoadLevel(const std::string& filepath)
{
    // Indeterminate: parsing a JSON file gives no intermediate progress to report, and a
    // bar sitting at 0% would suggest the load had stalled rather than that this stage
    // has nothing to measure.
    ReportLoadProgress(-1.0f, "Reading level");

    ReleaseCurrentLevel();
    
    //Find Level Json File
    std::ifstream level(filepath);
    if (!level.is_open())
    {
        System::GetInstance().DisplayMessageBox("Error", "Unable to open file for reading: " + filepath);
        return false;
    }

    nlohmann::json data;
    level >> data;
    level.close();
    
    //Create Level Object
    _currentLevel = new Level();
    _currentLevel->_path = filepath;
    _currentLevel->Load(data);

    ReportLoadProgress(1.0f, "Level ready");
    
    return true;
}

bool Engine::LevelManager::OpenLevel(const std::string& filepath)
{
    if (!LoadLevel(filepath)) return false;

    // Start is private on Level and this class is its friend, which is the whole reason
    // this function exists rather than the caller doing both halves itself.
    _currentLevel->Start();

    return true;
}

bool Engine::LevelManager::LoadLevel(NetCode::InputByteStream& stream)
{
    // The world arriving over the wire, which is how a client gets its level. Reported
    // for the same reason the file path is: this is the slow part of joining a server,
    // and it is the part a player is staring at a loading screen through.
    ReportLoadProgress(-1.0f, "Receiving world state");

    ReleaseCurrentLevel();
    _currentLevel = new Level();
    _currentLevel->Load(stream);

    ReportLoadProgress(1.0f, "World received");

    return true;
}
