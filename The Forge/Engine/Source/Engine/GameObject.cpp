#include "GameObject.h"

#include "Components/Component.h"
#include "JsonKeywords.h"
#include "Level.h"

Engine::GameObject::GameObject()
{
    _defaultData = {
        {JsonKeywords::GAMEOBJECT_NAME, "New Game Object"}
    };

    _name = "New Game Object";
}

Engine::GameObject::GameObject(const nlohmann::json& data)
{
    _name = data[JsonKeywords::GAMEOBJECT_NAME];

    if (!data.contains(JsonKeywords::COMPONENT_ARRAY)) return;

    for (const auto& component_data : data[JsonKeywords::COMPONENT_ARRAY])
    {
        //const auto ID = component_data[JsonKeywords::COMPONENT_ID];
        //TODO: Load Components
    }
}

Engine::Component* Engine::GameObject::AddComponentByID(uint32_t id)
{
    std::type_index type = GetComponentRegistry().GetComponentTypeFromID(id);
    //auto* pool = GetComponentManager().GetPool<id>();

    return nullptr;
}

std::vector<Engine::Component*> Engine::GameObject::GetAllComponents() const
{
    std::vector<Component*> componentList;
    for (const auto& val : _components | std::views::values) 
        componentList.push_back(val);
    
    return componentList;
}

nlohmann::json Engine::GameObject::SaveObject()
{
    nlohmann::json data;
    data[JsonKeywords::GAMEOBJECT_NAME] = _name;
    //TODO: save transform data (position & rotation)
    data[JsonKeywords::COMPONENT_ARRAY] = json::array();
    for (const auto& component : GetAllComponents())
        data[JsonKeywords::COMPONENT_ARRAY].push_back(component->SaveData());

    return data;
}
