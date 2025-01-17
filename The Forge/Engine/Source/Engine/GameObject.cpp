#include "GameObject.h"

#include "Component.h"
#include "ComponentUtils.h"
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
        ComponentUtils::AddComponent(this, component_data);
}

Engine::GameObject::~GameObject()
{
    for (const auto component : _attachedComponents)
        component->CleanUp();
    
    _attachedComponents.clear();
    _attachedComponents.shrink_to_fit();
}

void Engine::GameObject::AddComponent(Component* component)
{
    _attachedComponents.push_back(component);
}

nlohmann::json Engine::GameObject::SaveObject()
{
    json data = Object::SaveObject();
    data[JsonKeywords::COMPONENT_ARRAY] = json::array();
    for (const auto& component : _attachedComponents)
    {
        data[JsonKeywords::COMPONENT_ARRAY].push_back(component->SaveData());
    }

    return data;
}