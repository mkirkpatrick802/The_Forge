#include "ComponentRegistry.h"

std::shared_ptr<Engine::ComponentRegistry> Engine::ComponentRegistry::_instance = nullptr;
std::shared_ptr<Engine::ComponentRegistry> Engine::ComponentRegistry::GetInstance()
{
    _instance = _instance == nullptr ? std::make_shared<ComponentRegistry>() : _instance;
    return _instance;
}

uint32_t Engine::ComponentRegistry::GetComponentID(const std::type_info& typeInfo) const
{
    return _componentIDs.at(typeInfo);
}

std::string Engine::ComponentRegistry::GetComponentName(const std::type_info& typeInfo) const
{
    return _componentNames.at(typeInfo);
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
