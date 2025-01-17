#pragma once
#include <json.hpp>

#include "GameObject.h"
#include "Editor/Details.h"

#define PLAYER_CONTROLLER (uint32)0
#define SPRITE_RENDERER (uint32)1
#define PROJECTILE (uint32)2
#define COLLIDER (uint32)3
#define HEALTH (uint32)4
#define ENEMY_CONTROLLER (uint32)5
#define CAMERA (uint32)6

using json = nlohmann::json;

namespace Engine
{
    #define DECLARE_COMPONENT() \
    String GetName() const override { \
    String fullName = typeid(*this).name(); \
    return ExtractClassName(fullName); \
    }
    
    class GameObject;
    class Component : public Editor::Details
    {
    public:
        virtual ~Component() = default;

        virtual void LoadData(const json& data) = 0;
        virtual nlohmann::json SaveData() = 0;

        virtual void BeginPlay();
        virtual void Update(float deltaTime);

        virtual void OnDestroyed();
        virtual void CleanUp() = 0;

        virtual uint32 GetComponentID() const = 0;

        static const String& GetComponentNameFromID(uint32);
        static void ClearComponentNames() { _componentNames.clear(); }
    protected:

        static void RegisterComponent(uint32 id, const String& name);
        
    public:
        GameObject* gameObject = nullptr;

    private:

        inline static std::unordered_map<uint32, String> _componentNames;

    };

    inline void Component::BeginPlay() {}
    inline void Component::Update(float deltaTime) {}
    inline void Component::OnDestroyed(){}

    inline void Component::RegisterComponent(uint32 id, const String& name)
    {
        _componentNames[id] = name;
    }

    inline const String& Component::GetComponentNameFromID(const uint32 id)
    {
        return _componentNames[id];
    }
}