#include "ComponentUtils.h"

#include "Component.h"
#include "EventData.h"
#include "EventSystem.h"
#include "JsonKeywords.h"

std::unordered_map<uint32, Engine::componentFn> Engine::ComponentUtils::_componentCreationMap;
Engine::ComponentUtils* Engine::ComponentUtils::_instance = nullptr;

// Component Creations
Engine::ComponentUtils::ComponentUtils()
{
    RegisterComponentFns();
    _instance = this;
}

Engine::ComponentUtils::~ComponentUtils()
{
    _componentCreationMap.clear();
    _instance = nullptr;
}

void Engine::ComponentUtils::RegisterComponentFns()
{
    _componentCreationMap[SPRITE_RENDERER] = &ComponentUtils::CreateSpriteRenderer;
}

void Engine::ComponentUtils::AddComponent(GameObject* go, const nlohmann::json& data)
{
    const int componentID = data[JsonKeywords::COMPONENT_ID];
    const auto it = _componentCreationMap.find(componentID);
    if (it == _componentCreationMap.end())
        assert(0 && "Could not find valid component function");

    (_instance->*(it->second))(go, data);
}

void Engine::ComponentUtils::AddComponent(GameObject* go, uint32 id)
{
    const auto it = _componentCreationMap.find(id);
    if (it == _componentCreationMap.end())
        assert(0 && "Could not find valid component function");

    (_instance->*(it->second))(go, nullptr);
}

void Engine::ComponentUtils::CreateSpriteRenderer(GameObject* go, const nlohmann::json& data)
{
    ED_CreateSpriteRenderer eventData;
    eventData.gameObject = go;
    eventData.data = data;
    EventSystem::GetInstance()->TriggerEvent(ED_CreateSpriteRenderer::GetEventName(), &eventData);
}
