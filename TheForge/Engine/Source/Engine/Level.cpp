﻿#include "Level.h"
#include <fstream>

#include "CommandRegistry.h"
#include "GameModeBase.h"
#include "Components/Component.h"
#include "GameObject.h"
#include "JsonKeywords.h"
#include "LinkingContext.h"
#include "System.h"
#include "Components/Transform.h"

Engine::Level::Level()
{
    CommandRegistry::RegisterCommand("/save", [this](const std::string& args){ SaveLevel(args); });
    CommandRegistry::RegisterCommand("/gocount", [this](const std::string& args)
    {
        DEBUG_LOG("%d Game Objects", _gameObjects.size())
    });
}

Engine::Level::~Level()
{
    CommandRegistry::UnregisterCommand("/save");
    CommandRegistry::UnregisterCommand("/gocount");
    
    _gameObjects.clear();
}

void Engine::Level::Load(nlohmann::json data)
{
    _name = data[JsonKeywords::LEVEL_NAME];
    if (data.contains("Size X") && data.contains("Size Y"))
    {
        _size.x = data["Size X"];
        _size.y = data["Size Y"];
    }

    //Load Game Objects From Json Data
    if (!data.contains(JsonKeywords::GAMEOBJECT_ARRAY)) return;

    for (const auto& go_data : data[JsonKeywords::GAMEOBJECT_ARRAY])
    {
        auto go = std::make_unique<GameObject>(go_data);
        _gameObjects.push_back(std::move(go));
    }

    _gameMode = std::make_unique<GameModeBase>();
}

void Engine::Level::Load(NetCode::InputByteStream& stream)
{
    Read(stream);
}

void Engine::Level::Start() const
{
    _gameMode->Start();
}

Engine::GameObject* Engine::Level::SpawnNewGameObject(const std::string& filepath, const glm::vec2 position)
{
    // Create Game Object
    auto go = std::make_unique<GameObject>();
    go->isDirty = true;
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
    else
    {
        go->AddComponent<Transform>();
    }

    go->SetPosition(position);
    
    // Move the unique_ptr into the vector
    auto* rawPtr = go.get();
    _gameObjects.push_back(std::move(go));

    // Return the raw pointer which now safely points to the object in the vector
    return rawPtr;
}

Engine::GameObject* Engine::Level::SpawnNewGameObjectFromJson(const nlohmann::json& data)
{
    auto go = std::make_unique<GameObject>();
    go->isDirty = true;
    go->Deserialize(data);

    // Move the unique_ptr into the vector
    auto* rawPtr = go.get();
    _gameObjects.push_back(std::move(go));

    // Return the raw pointer which now safely points to the object in the vector
    return rawPtr;
}

Engine::GameObject* Engine::Level::SpawnNewGameObjectFromInputStream(NetCode::InputByteStream& stream, const uint32_t NID)
{
    auto newGo = std::make_unique<GameObject>();
    NetCode::GetLinkingContext().AddGameObject(newGo.get(), NID);
    newGo->Read(stream);
    
    auto* rawPtr = newGo.get();
    _gameObjects.push_back(std::move(newGo));
    return rawPtr;
}

bool Engine::Level::RemoveGameObject(GameObject* go, bool replicate)
{
    for (auto it = _gameObjects.begin(); it != _gameObjects.end(); )
    {
        if (it->get() == go) // Compare raw pointers
        {
            if (replicate)
                _destroyedObjects.push_back(go); // Move ownership to _destroyedObjects

            if(auto parent = go->GetParent())
            {
                parent->RemoveChild(go);
            }
            
            it = _gameObjects.erase(it); // Erase and update iterator
        }
        else
        {
            ++it;
        }
    }

    return true;
}

void Engine::Level::SaveLevel(const std::string& args)
{
    nlohmann::json data;
    
    // Update level data
    data[JsonKeywords::LEVEL_NAME] = _name;
    data["Size X"] = _size.x;
    data["Size Y"] = _size.y;

    // Update game objects
    data[JsonKeywords::GAMEOBJECT_ARRAY] = json::array();
    for (const auto& go : _gameObjects)
        if(!go->GetParent())
            data[JsonKeywords::GAMEOBJECT_ARRAY].push_back(go->Serialize());

    // Write new data
    if (std::ofstream outputFile(_path); outputFile.is_open())
    {
        outputFile << data.dump(4);
        outputFile.close();
    }

    std::cout << "Level Saved Successfully!" << '\n';
}

void Engine::Level::Write(NetCode::OutputByteStream& stream, bool isCompleteState)
{
    // Count replicated objects that need to be sent
    uint32_t replicatedCount = 0;
    for (const auto& element : _gameObjects)
    {
        if ((!element->isServerOnly && isCompleteState) || (element->isReplicated && element->isDirty))
        {
            ++replicatedCount;
        }
    }

    // Write the data for each replicated object
    stream.Write(replicatedCount);
    if (replicatedCount > 0)
    {
        for (auto& element : _gameObjects)
        {
            if ((!element->isServerOnly && isCompleteState) || (element->isReplicated && element->isDirty))
            {
                const uint32_t networkID = NetCode::GetLinkingContext().GetNetworkID(element.get());
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

    uint32_t destroyedCount = (uint32_t)_destroyedObjects.size();
    stream.Write(destroyedCount);
    for (auto& element : _destroyedObjects)
    {
        const uint32_t networkID = NetCode::GetLinkingContext().GetNetworkID(element, false);
        stream.Write(networkID);
        NetCode::GetLinkingContext().RemoveGameObject(element);
    }
    _destroyedObjects.clear();
}

void Engine::Level::Read(NetCode::InputByteStream& stream)
{
    uint32_t elementCount;
    stream.Read(elementCount);
    for (int i = 0; i < (int)elementCount; i++)
    {
        uint32_t networkID;
        stream.Read(networkID);
        if (const auto go = NetCode::GetLinkingContext().GetGameObject(networkID); go == nullptr)
        {
            const auto newGo = SpawnNewGameObjectFromInputStream(stream, networkID);
            DEBUG_LOG("Creating game object: %s", newGo->GetName().c_str())
        }
        else
        {
            go->Read(stream);
        }
    }

    uint32_t destroyedCount;
    stream.Read(destroyedCount);
    for (int i = 0; i < (int)destroyedCount; i++)
    {
        uint32_t networkID;
        stream.Read(networkID);
        RemoveGameObject(NetCode::GetLinkingContext().GetGameObject(networkID), false);
    }

    for (const auto& go : _gameObjects)
    {
        go->LinkFamily();
    }
}

std::vector<Engine::GameObject*> Engine::Level::GetAllGameObjects() const
{
    std::vector<GameObject*> result;
    result.reserve(_gameObjects.size());  // Optimize memory allocation
    for (auto& obj : _gameObjects) {
        result.push_back(obj.get());  // Push raw pointers to GameObject
    }
    return result;
}
