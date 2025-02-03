#pragma once
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>

#define REGISTER_COMPONENT(TYPE) \
class TYPE##RegisterHelper { \
public: \
TYPE##RegisterHelper() { \
ComponentRegistry::GetInstance()->RegisterComponent<TYPE>(#TYPE); \
} \
}; \
static TYPE##RegisterHelper TYPE##_register_helper;

namespace Engine
{
    class ComponentRegistry
    {
    public:
        
        static std::shared_ptr<ComponentRegistry> GetInstance();
        
        template <typename T>
        void RegisterComponent(const std::string& name);

        template <typename T>
        uint32_t GetComponentID() const;
        uint32_t GetComponentID(const std::type_info& typeInfo) const;

        template <typename T>
        std::string GetComponentName() const;
        std::string GetComponentName(const std::type_info& typeInfo) const;

        std::vector<std::pair<std::string, uint32_t>> GetListOfComponents() const;

    private:

        std::string FormatComponentName(const std::string& name) const;
        
    private:

        static std::shared_ptr<ComponentRegistry> _instance;
        
        std::unordered_map<std::type_index, uint32_t> _componentIDs;
        std::unordered_map<std::type_index, std::string> _componentNames;
    };

    template <typename T>
    void ComponentRegistry::RegisterComponent(const std::string& name)
    {
        const uint32_t id = static_cast<uint32_t>(std::hash<std::string>{}(name));
        
        _componentIDs[typeid(T)] = id;
        _componentNames[typeid(T)] = FormatComponentName(name);
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

    // Global accessor for the component registry
    inline std::shared_ptr<ComponentRegistry> GetComponentRegistry()
    {
        return ComponentRegistry::GetInstance();
    }
}
