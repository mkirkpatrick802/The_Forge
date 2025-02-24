#pragma once
#include <cstdint>
#include <memory>
#include <unordered_map>

namespace Engine
{
    class GameObject;
}

namespace NetCode
{
#define NULL_ID 0
    class LinkingContext
    {
    public:
        static LinkingContext& GetInstance();
        LinkingContext();
        uint32_t GetNetworkID(Engine::GameObject* go, bool shouldCreateIfNotFound = true);

        void AddGameObject(Engine::GameObject* go, uint32_t networkID);
        void RemoveGameObject(const Engine::GameObject* go);
        Engine::GameObject* GetGameObject(uint32_t networkID);
    
    private:
        std::unordered_map<uint32_t, Engine::GameObject*> _goMap;
        std::unordered_map<const Engine::GameObject*, uint32_t> _networkIDMap;
        uint32_t _nextNetworkID = 1;
    };

    inline LinkingContext& GetLinkingContext()
    {
        return LinkingContext::GetInstance();
    }
}
