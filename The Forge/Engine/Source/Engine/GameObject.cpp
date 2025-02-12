#include "GameObject.h"

#include "Components/Component.h"
#include "JsonKeywords.h"
#include "Level.h"
#include "LinkingContext.h"
#include "Components/ComponentFactories.h"
#include "Components/ComponentRegistry.h"

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
    
    if (data.contains(JsonKeywords::GAMEOBJECT_ISREPLICATED))
        isReplicated = data[JsonKeywords::GAMEOBJECT_ISREPLICATED];

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
    data[JsonKeywords::GAMEOBJECT_ISREPLICATED] = isReplicated;
    
    //TODO: save transform data (position & rotation)
    data[JsonKeywords::COMPONENT_ARRAY] = json::array();
    for (const auto& component : GetAllComponents())
        data[JsonKeywords::COMPONENT_ARRAY].push_back(component->Serialize());

    return data;
}

void Engine::GameObject::Write(NetCode::OutputByteStream& stream)
{
    stream.Write(_name);
    stream.Write(id);
    stream.Write(transform.position);
    stream.Write(transform.rotation);

    uint32_t componentCount = static_cast<uint32_t>(_components.size());
    stream.Write(componentCount);
    for (const auto& val : _components | std::views::values)
    {
        uint32_t componentID = GetComponentRegistry().GetComponentID(typeid(*val));
        stream.Write(componentID);
        val->Write(stream);
    }
}

void Engine::GameObject::Read(NetCode::InputByteStream& stream)
{
    stream.Read(_name);
    stream.Read(id);
    stream.Read(transform.position);
    stream.Read(transform.rotation);

    uint32_t componentCount;
    stream.Read(componentCount);
    for (uint32_t i = 0; i < componentCount; i++)
    {
        uint32_t componentID;
        stream.Read(componentID);
        auto type = GetComponentRegistry().GetComponentTypeFromID(componentID);
        if (auto it = _components.find(type); it == _components.end())
        {
            const auto component = GetComponentFactories().CreateComponentFromID(componentID, this);
            component->Read(stream);
        }
        else
        {
            it->second->Read(stream);
        }
    }
}
