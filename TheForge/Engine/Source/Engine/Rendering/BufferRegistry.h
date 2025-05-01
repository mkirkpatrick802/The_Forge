#pragma once
#include <unordered_map>

#include "Framebuffer.h"

namespace Engine
{
    class BufferRegistry
    {
    public:
        
        friend class Renderer;   
        enum class BufferType : uint8_t { SCENE = 0 };
        
    public:

        BufferRegistry() = default;
        static std::shared_ptr<BufferRegistry> GetRegistry();
        std::shared_ptr<Framebuffer> GetBuffer(BufferType type) const;
        
    private:
        

        void AddBuffer(BufferType type, std::shared_ptr<Framebuffer> buffer);
        void CleanUp() const;
        
    private:
        
        static std::shared_ptr<BufferRegistry> _registry;
        std::unordered_map<BufferType, std::shared_ptr<Framebuffer>> _buffers;
    };
}
