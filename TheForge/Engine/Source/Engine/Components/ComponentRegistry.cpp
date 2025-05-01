#include "ComponentRegistry.h"

#include <stdexcept>

Engine::ComponentRegistry& Engine::ComponentRegistry::GetInstance()
{
    static auto instance = std::make_unique<ComponentRegistry>();
    return *instance;
}

uint32_t Engine::ComponentRegistry::GetComponentID(const std::type_info& typeInfo) const
{
    if (const auto it = _componentIDs.find(typeInfo); it != _componentIDs.end())
        return it->second;

    throw std::runtime_error("Component Type not found!");
}

std::string Engine::ComponentRegistry::GetComponentName(const std::type_info& typeInfo) const
{
    if (const auto it = _componentNames.find(typeInfo); it != _componentNames.end())
        return it->second;

    throw std::runtime_error("Component Type not found!");
}

std::type_index Engine::ComponentRegistry::GetComponentTypeFromID(const uint32_t id) const
{
    if (const auto it = _componentTypes.find(id); it != _componentTypes.end()) 
        return it->second;
    
    throw std::runtime_error("Component ID not found!");
}

std::vector<std::pair<std::string, uint32_t>> Engine::ComponentRegistry::GetListOfComponents() const
{
    std::vector<std::pair<std::string, uint32_t>> result;

    // Iterate over _componentNames and _componentIDs to create the pairs
    for (const auto& [fst, snd] : _componentNames)
    {
        const auto& name = snd;
        if (auto idIter = _componentIDs.find(fst); idIter != _componentIDs.end())
        {
            result.emplace_back(name, idIter->second);
        }
    }

    return result;
}

std::string Engine::ComponentRegistry::FormatComponentName(const std::string& name) const
{
    std::string formattedName;
    for (size_t i = 0; i < name.size(); ++i) {
        if (i > 0 && std::isupper(name[i]) && std::islower(name[i - 1])) {
            formattedName += ' ';
        }
        formattedName += name[i];
    }
    return formattedName;
}
