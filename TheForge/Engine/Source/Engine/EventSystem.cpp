#include "EventSystem.h"

#include <ranges>

std::shared_ptr<Engine::EventSystem> Engine::EventSystem::_instance = nullptr;

std::shared_ptr<Engine::EventSystem> Engine::EventSystem::GetInstance()
{
    return _instance ? _instance : (_instance = std::make_shared<EventSystem>());
}

void Engine::EventSystem::DestroyInstance()
{
    _instance.reset();
}

void Engine::EventSystem::RegisterEvent(const std::string& name, void* obj, const EventCallback& callback)
{
    //const auto pair = std::make_pair(obj, name);
    _eventMap[name][obj].push_back(callback);
}

void Engine::EventSystem::TriggerEvent(const std::string& name, const void* data)
{
    if (const auto it = _eventMap.find(name); it != _eventMap.end()) {
        for (auto& callbacks : it->second | std::views::values) {
            for (auto& callback : callbacks) {
                callback(data); // Trigger the callback
            }
        }
    }
}

void Engine::EventSystem::DeregisterEvent(const std::string& name, void* obj)
{
    _eventMap[name].erase(obj);

    // If no more objects are registered for this event, remove the event entry
    if (_eventMap[name].empty()) {
        _eventMap.erase(name);
    }
}
