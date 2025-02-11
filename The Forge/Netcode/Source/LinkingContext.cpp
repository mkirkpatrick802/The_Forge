#include "LinkingContext.h"

#include <memory>

NetCode::LinkingContext& NetCode::LinkingContext::GetInstance()
{
    static auto instance = std::make_unique<LinkingContext>();
    return *instance;
}

NetCode::LinkingContext::LinkingContext(): _nextNetworkID(0) { }

uint32_t NetCode::LinkingContext::GetNetworkID(Engine::GameObject* go, const bool shouldCreateIfNotFound)
{
    if (const auto it = _networkIDMap.find(go); it != _networkIDMap.end())
        return it->second;
    
    if (shouldCreateIfNotFound)
    {
        const uint32_t networkID = _nextNetworkID++;
        AddGameObject(go, networkID);
        return networkID;
    }
    
    return 0;
}

void NetCode::LinkingContext::AddGameObject(Engine::GameObject* go, const uint32_t networkID)
{
    _networkIDMap[go] = networkID;
    _goMap[networkID] = go;
}

void NetCode::LinkingContext::RemoveGameObject(const Engine::GameObject* go)
{
    const uint32_t networkID = _networkIDMap[go];
    _networkIDMap.erase(go);
    _goMap.erase(networkID);
}

Engine::GameObject* NetCode::LinkingContext::GetGameObject(const uint32_t networkID)
{
    if (const auto it = _goMap.find(networkID); it != _goMap.end())
        return it->second;

    return nullptr;
}
