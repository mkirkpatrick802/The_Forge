#include "EventSystem.h"

Engine::EventSystem* Engine::EventSystem::_instance = nullptr;

Engine::EventSystem* Engine::EventSystem::GetInstance()
{
    return _instance ? _instance : (_instance = DEBUG_NEW EventSystem());
}

void Engine::EventSystem::DestroyInstance()
{
    delete _instance;
    _instance = nullptr;
}

void Engine::EventSystem::RegisterEvent(const String& name, const EventCallback& callback)
{
    _eventMap[name].push_back(callback);
}

void Engine::EventSystem::TriggerEvent(const String& name, const void* data)
{
    if (const auto it = _eventMap.find(name); it != _eventMap.end())
    {
        for (const auto& callback : it->second)
        {
            callback(data);
        }
    }
}

void Engine::EventSystem::DeregisterEvent(const String& name)
{
    _eventMap.erase(name);
}
