#include "LevelManager.h"

#include <fstream>
#include <iostream>

#include "json.hpp"
#include "JsonKeywords.h"
#include "Level.h"
#include "System.h"

const std::string Engine::LevelManager::LEVEL_PATH = "Assets/Levels/";
Engine::Level* Engine::LevelManager::_currentLevel = nullptr;

Engine::LevelManager::LevelManager(const std::string& filepath)
{
    _componentUtils = std::make_unique<ComponentUtils>();
    
    if (const std::ifstream file(filepath); file.is_open())
        LoadLevel(filepath);
}

void Engine::LevelManager::CleanUp()
{
    delete _currentLevel;
    _currentLevel = nullptr;
    
    _componentUtils.reset();
}

bool Engine::LevelManager::CreateLevel(const String& levelName)
{
    const String filepath = LEVEL_PATH + levelName + ".json";

    if (std::filesystem::exists(filepath))
    {
        System::DisplayMessageBox("Error", "File already exists!");
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
    //Find Level Json File
    std::ifstream level(filepath);
    if (!level.is_open())
    {
        System::DisplayMessageBox("Error", "Unable to open file for reading: " + filepath);
        return false;
    }

    nlohmann::json data;
    level >> data;
    
    //Create Level Object
    delete _currentLevel;
    _currentLevel = DEBUG_NEW Level(data);
    _currentLevel->_path = filepath;
    
    return true;
}
