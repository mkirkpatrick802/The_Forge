#include "EventSystem.h"

std::shared_ptr<Engine::EventSystem> Engine::EventSystem::_instance = nullptr;

std::shared_ptr<Engine::EventSystem> Engine::EventSystem::GetInstance()
{
    return _instance ? _instance : (_instance = std::make_shared<EventSystem>());
}

void Engine::EventSystem::DestroyInstance()
{
    _instance.reset();
}

void Engine::EventSystem::RegisterEvent(const std::string& name, const EventCallback& callback)
{
    _eventMap[name].push_back(callback);
}

void Engine::EventSystem::TriggerEvent(const std::string& name, const void* data)
{
    if (const auto it = _eventMap.find(name); it != _eventMap.end())
    {
        for (const auto& callback : it->second)
        {
            callback(data);
        }
    }
}

void Engine::EventSystem::DeregisterEvent(const std::string& name)
{
    _eventMap.erase(name);
}
