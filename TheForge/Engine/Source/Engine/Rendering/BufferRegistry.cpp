#include "BufferRegistry.h"

#include <utility>


std::shared_ptr<Engine::BufferRegistry> Engine::BufferRegistry::_registry;

std::shared_ptr<Engine::BufferRegistry> Engine::BufferRegistry::GetRegistry()
{
    if (_registry == nullptr)
        _registry = std::make_shared<BufferRegistry>();
    
    return _registry;
}

void Engine::BufferRegistry::AddBuffer(const BufferType type, std::shared_ptr<Framebuffer> buffer)
{
    _buffers[type] = std::move(buffer);
}

std::shared_ptr<Engine::Framebuffer> Engine::BufferRegistry::GetBuffer(const BufferType type) const
{
    return _buffers.at(type);
}

void Engine::BufferRegistry::CleanUp() const
{
    _registry.reset();
}
