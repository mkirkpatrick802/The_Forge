#pragma once
#include <cstdint>
#include <functional>
#include <memory>
#include <ranges>
#include <string>
#include <typeindex>
#include <unordered_map>

#include "ComponentHasher.h"

namespace Engine
{
    class ComponentRegistry
    {
    public:
        static ComponentRegistry& GetInstance();
        
        template <typename T>
        void RegisterComponent(const std::string& name);

        template <typename T>
        uint32_t GetComponentID() const;
        uint32_t GetComponentID(const std::type_info& typeInfo) const;

        template <typename T>
        std::string GetComponentName() const;
        std::string GetComponentName(const std::type_info& typeInfo) const;

        std::type_index GetComponentTypeFromID(const uint32_t id) const;

        std::vector<std::pair<std::string, uint32_t>> GetListOfComponents() const;
        
    private:
        std::string FormatComponentName(const std::string& name) const;
        
    private:
        std::unordered_map<uint32_t, std::type_index> _componentTypes;
        std::unordered_map<std::type_index, uint32_t> _componentIDs;
        std::unordered_map<std::type_index, std::string> _componentNames;

    };

    template <typename T>
    void ComponentRegistry::RegisterComponent(const std::string& name)
    {
        const uint32_t id = HASH_COMPONENT(name);
        std::type_index type = typeid(T);
        std::string formattedName = FormatComponentName(name);

        _componentIDs[type] = id;
        _componentNames[type] = formattedName;
        _componentTypes.insert({id, type});
    }

    template <typename T>
    uint32_t ComponentRegistry::GetComponentID() const
    {
        return _componentIDs.at(typeid(T));
    }

    template <typename T>
    std::string ComponentRegistry::GetComponentName() const
    {
        return _componentNames.at(typeid(T));
    }

    inline ComponentRegistry& GetComponentRegistry()
    {
        return ComponentRegistry::GetInstance();
    }
}
