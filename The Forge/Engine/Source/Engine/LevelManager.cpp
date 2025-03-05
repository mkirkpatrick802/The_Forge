#include "LevelManager.h"

#include <fstream>
#include <iostream>

#include "EngineManager.h"
#include "json.hpp"
#include "JsonKeywords.h"
#include "Level.h"
#include "System.h"
#include "Components/ComponentRegistry.h"
#include "Editor/DetailsEditor.h"
#include "Rendering/CameraManager.h"

Engine::Level* Engine::LevelManager::_currentLevel = nullptr;

Engine::LevelManager::LevelManager(const std::string& filepath)
{
    if (const std::ifstream file(filepath); file.is_open())
        LoadLevel(filepath);

    if (!GetEngineManager().IsEditorEnabled())
        StartCurrentLevel();
}

void Engine::LevelManager::StartCurrentLevel() const
{
    Editor::DetailsEditor::ClearSelectedPrefab();
    
    GetCurrentLevel()->SaveLevel();
    GetEngineManager().ToggleEditor("0");

    // Need to make a copy of the file path because it will get deleted in the LoadLevel function
    const std::string path = GetCurrentLevel()->_path;
    LoadLevel(path);
    
    const auto cameras = GetComponentManager().GetPool<Camera>()->GetActive();
    if (!cameras.empty())
        GetCameraManager().SetActiveCamera(cameras[0]);
    
    _currentLevel->Start();
}

void Engine::LevelManager::CleanUp() const
{
    delete _currentLevel;
    _currentLevel = nullptr;
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
    delete _currentLevel;
    
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
    
    return true;
}

bool Engine::LevelManager::LoadLevel(NetCode::InputByteStream& stream)
{
    delete _currentLevel;
    _currentLevel = new Level();
    _currentLevel->Load(stream);

    return true;
}
