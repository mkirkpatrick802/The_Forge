#pragma once
#include "Engine/Data.h"
#include "json.hpp"

namespace Engine
{
    class GameObject;
    class ComponentUtils;
    typedef void (ComponentUtils::* componentFn)(GameObject* go, const nlohmann::json& data);
    class ComponentUtils
    {
    public:
        ComponentUtils();
        ~ComponentUtils();
        
        static void AddComponent(GameObject* go, const nlohmann::json& data);
        static void AddComponent(GameObject* go, uint32 id);

    private:

        void RegisterComponentFns();
        void CreateSpriteRenderer(GameObject* go, const nlohmann::json& data = nullptr);
        void CreateCamera(GameObject* go, const nlohmann::json& data = nullptr);

    private:

        static std::unordered_map<uint32, componentFn> _componentCreationMap;
        static ComponentUtils* _instance;
    };
}
