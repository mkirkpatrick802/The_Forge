#include "GameObject.h"

#include "Components/Component.h"
#include "JsonKeywords.h"
#include "Level.h"
#include "LinkingContext.h"
#include "Components/ComponentFactories.h"
#include "Components/ComponentRegistry.h"
#include "Components/Transform.h"

Engine::GameObject::GameObject(): _parent(nullptr)
{
    _defaultData = {
        {JsonKeywords::GAMEOBJECT_NAME, "New Game Object"}
    };

    _name = "New Game Object";
}

Engine::GameObject::GameObject(const nlohmann::json& data): _parent(nullptr)
{
    _components.reserve(1);
    Deserialize(data);
}

Engine::GameObject::~GameObject()
{
    for (const auto val : _components | std::views::values)
    {
        if (val)
            GetComponentFactories().DeleteComponent(val);
    }

    _components.clear();
}

std::vector<Engine::Component*> Engine::GameObject::GetAllComponents() const
{
    std::vector<Component*> componentList;
    if (_components.empty()) return componentList;
    for (const auto& val : _components | std::views::values) 
        componentList.push_back(val);
    
    return componentList;
}

void Engine::GameObject::RemoveComponent(Component* component)
{
    if (!component) return;
    const std::type_index type = typeid(*component);  // Get the actual type of the component

    if (const auto it = _components.find(type); it != _components.end() && it->second == component)
        _components.erase(it);

    GetComponentFactories().DeleteComponent(component);
}

void Engine::GameObject::AddChild(GameObject* child, const bool keepWorldPosition)
{
    child->_parent = this;
    _children.push_back(child);
    
    if(keepWorldPosition)
    {
        const glm::vec2 pos = child->GetWorldPosition();
        child->UpdateWorldTransform();
        child->SetPosition(pos - GetWorldPosition());
    }
    else
    {
        child->UpdateWorldTransform();
    }
}

void Engine::GameObject::RemoveChild(GameObject* child)
{
    if (const auto it = std::ranges::find(_children, child); it != _children.end())
    {
        _children.erase(it);
        child->_parent = nullptr; // Remove the parent reference
    }
}

void Engine::GameObject::UpdateWorldTransform() const
{
    if(const auto transform = GetComponent<Transform>())
    {
        transform->UpdateWorldTransform(_parent ? _parent->GetComponent<Transform>() : nullptr);
    }

    for (const auto child : _children)
        child->UpdateWorldTransform();
}

void Engine::GameObject::SetPosition(const glm::vec2& position) const
{
    if (const auto transform = GetComponent<Transform>())
        transform->SetLocalPosition(position);

    UpdateWorldTransform();
}

glm::vec2 Engine::GameObject::GetWorldPosition() const
{
    if (const auto transform = GetComponent<Transform>())
        return transform->GetWorldPosition();

    return glm::vec2(0.0f);
}

glm::vec2 Engine::GameObject::GetLocalPosition() const
{
    if (const auto transform = GetComponent<Transform>())
        return transform->GetLocalPosition();

    return glm::vec2(0.0f); 
}

void Engine::GameObject::SetRotation(const float rotation) const
{
    if (const auto transform = GetComponent<Transform>())
        transform->SetLocalRotation(rotation);

    UpdateWorldTransform();
}

float Engine::GameObject::GetWorldRotation() const
{
    if (const auto transform = GetComponent<Transform>())
        return transform->GetWorldRotation();

    return 0.0f;
}

void Engine::GameObject::Deserialize(const nlohmann::json& data)
{
    _name = data[JsonKeywords::GAMEOBJECT_NAME];
    
    if (data.contains(JsonKeywords::GAMEOBJECT_ISREPLICATED))
        isReplicated = data[JsonKeywords::GAMEOBJECT_ISREPLICATED];

    if (data.contains("Server Only"))
        isServerOnly = data["Server Only"];

    if (!data.contains(JsonKeywords::COMPONENT_ARRAY)) return;

    for (const auto& component_data : data[JsonKeywords::COMPONENT_ARRAY])
    {
        const uint32_t id = component_data[JsonKeywords::COMPONENT_ID];
        if (const auto component = GetComponentFactories().CreateComponentFromID(id, this); component != nullptr)
            component->Deserialize(component_data);
        else
        {
            std::cout << "ERROR: Could not create component from ID" << id << '\n';
            std::cout << "Name: " << GetComponentRegistry().GetComponentTypeFromID(id).name() << '\n';
        }
    }

    if(data.contains("Children"))
    {
        for (const auto& child_data : data["Children"])
        {
            GameObject* child = LevelManager::GetCurrentLevel()->SpawnNewGameObjectFromJson(child_data);
            child->_parent = this;
            _children.push_back(child);
        }
    }

    UpdateWorldTransform();
}

nlohmann::json Engine::GameObject::Serialize()
{
    nlohmann::json data;
    data[JsonKeywords::GAMEOBJECT_NAME] = _name;
    data[JsonKeywords::GAMEOBJECT_ISREPLICATED] = isReplicated;
    data["Server Only"] = isServerOnly;
    
    data[JsonKeywords::COMPONENT_ARRAY] = json::array();
    for (const auto& component : GetAllComponents())
    {
        nlohmann::json componentData = component->Serialize();
        componentData[JsonKeywords::COMPONENT_ID] = GetComponentRegistry().GetComponentID(typeid(*component));
        data[JsonKeywords::COMPONENT_ARRAY].push_back(componentData);
    }
    
    data["Children"] = json::array();
    for (const auto& child : _children)
    {
        nlohmann::json childData = child->Serialize();
        data["Children"].push_back(childData);
    }

    return data;
}

void Engine::GameObject::Write(NetCode::OutputByteStream& stream) const
{
    stream.Write(_name);

    // Write Parent
    const uint32_t parentNID = NetCode::GetLinkingContext().GetNetworkID(_parent);
    stream.Write(parentNID);
    _parent->Write(stream);

    // Write Children
    uint32_t childCount = _children.size();
    stream.Write(childCount);
    for (const auto child : _children)
    {
        const uint32_t childNID = NetCode::GetLinkingContext().GetNetworkID(child);
        stream.Write(childNID);
        _parent->Write(stream);
    }

    // Write Components
    std::vector<Component*> replicatedComponents;
    for (const auto& component : GetAllComponents())
    {
        replicatedComponents.push_back(component);
    }
    
    uint32_t componentCount = static_cast<uint32_t>(replicatedComponents.size());
    stream.Write(componentCount);
    for (const auto& component : replicatedComponents)
    {
        uint32_t componentID = GetComponentRegistry().GetComponentID(typeid(*component));
        stream.Write(componentID);
        component->Write(stream);
    }
}

void Engine::GameObject::Read(NetCode::InputByteStream& stream)
{
    stream.Read(_name);

    // Read Parent
    uint32_t parentNID;
    stream.Read(parentNID);
    if (const auto go = NetCode::GetLinkingContext().GetGameObject(parentNID); go == nullptr)
    {
        _parent = LevelManager::GetCurrentLevel()->SpawnNewGameObjectFromInputStream(stream, parentNID);
    }
    else
    {
        go->Read(stream);
    }

    // Read Children
    uint32_t childCount;
    stream.Read(childCount);
    for (int i = 0; i < childCount; i++)
    {
        uint32_t childNID;
        stream.Read(childNID);
        if (const auto go = NetCode::GetLinkingContext().GetGameObject(parentNID); go == nullptr)
        {
            auto child = LevelManager::GetCurrentLevel()->SpawnNewGameObjectFromInputStream(stream, parentNID);
            _children.push_back(child);
        }
        else
        {
            go->Read(stream);
        }
    }

    // Read Components
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