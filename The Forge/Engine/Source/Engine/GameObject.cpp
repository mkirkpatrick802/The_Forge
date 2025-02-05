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
    Deserialize(data);
}

Engine::GameObject::~GameObject()
{
    for (const auto val : _components | std::views::values)
        GetComponentFactories().DeleteComponent(val);

    _components.clear();
}

std::vector<Engine::Component*> Engine::GameObject::GetAllComponents() const
{
    std::vector<Component*> componentList;
    for (const auto& val : _components | std::views::values) 
        componentList.push_back(val);
    
    return componentList;
}

void Engine::GameObject::Deserialize(const nlohmann::json& data)
{
    _name = data[JsonKeywords::GAMEOBJECT_NAME];

    if (!data.contains(JsonKeywords::COMPONENT_ARRAY)) return;

    for (const auto& component_data : data[JsonKeywords::COMPONENT_ARRAY])
    {
        const auto& id = component_data[JsonKeywords::COMPONENT_ID];
        const auto component = GetComponentFactories().CreateComponentFromID(id, this);
        component->Deserialize(component_data);
    }
}

nlohmann::json Engine::GameObject::Serialize()
{
    nlohmann::json data;
    data[JsonKeywords::GAMEOBJECT_NAME] = _name;
    //TODO: save transform data (position & rotation)
    data[JsonKeywords::COMPONENT_ARRAY] = json::array();
    for (const auto& component : GetAllComponents())
        data[JsonKeywords::COMPONENT_ARRAY].push_back(component->Serialize());

    return data;
}
