#include "Level.h"
#include <fstream>

#include "CommandRegistry.h"
#include "GameModeBase.h"
#include "Components/Component.h"
#include "GameObject.h"
#include "JsonKeywords.h"
#include "LinkingContext.h"
#include "System.h"

Engine::Level::Level(nlohmann::json data)
{
    CommandRegistry::RegisterCommand("/save", [this](const std::string& args){ SaveLevel(args); });
    
    _name = data[JsonKeywords::LEVEL_NAME];

    //Load Game Objects From Json Data
    if (!data.contains(JsonKeywords::GAMEOBJECT_ARRAY)) return;

    for (const auto& go_data : data[JsonKeywords::GAMEOBJECT_ARRAY])
    {
        const auto go = new GameObject(go_data);
        _gameObjects.push_back(go);
    }

    _gameMode = std::make_unique<GameModeBase>();
}

Engine::Level::~Level()
{
    CommandRegistry::UnregisterCommand("/save");
    
    // Clean Up all the game objects
    for (auto go : _gameObjects)
    {
        delete go;
        go = nullptr;
    }

    _gameObjects.clear();
}

void Engine::Level::Start()
{
    _gameMode->Start();
}

Engine::GameObject* Engine::Level::SpawnNewGameObject(const std::string& filepath)
{
    // Create Game Object
    const auto go = new GameObject();
    if (!filepath.empty())
    {
        std::ifstream file(filepath);
        if (!file.is_open())
        {
            std::cerr << "Failed to open file " << filepath << '\n';
            return nullptr;
        }
        
        json j;
        file >> j; // Parse JSON from the file stream
        go->Deserialize(j);
    }
    
    _gameObjects.push_back(go);

    return go;
}

bool Engine::Level::RemoveGameObject(GameObject* go)
{
    std::erase(_gameObjects, go);
    delete go;
    return true;
}

void Engine::Level::SaveLevel(const std::string& args)
{
    nlohmann::json data;
    
    // Update level data
    data[JsonKeywords::LEVEL_NAME] = _name;

    // Update game objects
    data[JsonKeywords::GAMEOBJECT_ARRAY] = json::array();
    for (const auto go : _gameObjects)
        data[JsonKeywords::GAMEOBJECT_ARRAY].push_back(go->Serialize());

    // Write new data
    if (std::ofstream outputFile(_path); outputFile.is_open())
    {
        outputFile << data.dump(4);
        outputFile.close();
    }

    std::cout << "Level Saved Successfully!" << '\n';
}

void Engine::Level::Write(NetCode::OutputByteStream& stream, bool isCompleteState) const
{
    // Count replicated objects that need to be sent
    uint32_t replicatedCount = 0;
    for (const auto& element : _gameObjects)
    {
        if (element->isReplicated && (isCompleteState || element->isDirty))
        {
            ++replicatedCount;
        }
    }

    stream.Write(replicatedCount);

    // Write the data for each replicated object
    for (GameObject* element : _gameObjects)
    {
        if (element->isReplicated && (isCompleteState || element->isDirty))
        {
            const uint32_t networkID = NetCode::GetLinkingContext().GetNetworkID(element);
            stream.Write(networkID);
            element->Write(stream);

            // If it's not a complete state, mark the object as clean
            if (!isCompleteState)
            {
                element->isDirty = false;
            }
        }
    }
}

void Engine::Level::Read(NetCode::InputByteStream& stream)
{
    uint32_t elementCount;
    stream.Read(elementCount);
    
    for (int i = 0; i < elementCount; i++)
    {
        uint32_t networkID;
        stream.Read(networkID);
        auto go = NetCode::GetLinkingContext().GetGameObject(networkID);
        if (go == nullptr)
        {
            go = new GameObject();
            NetCode::GetLinkingContext().AddGameObject(go, networkID);
            _gameObjects.push_back(go);
        }
        
        go->Read(stream);
    }
}
