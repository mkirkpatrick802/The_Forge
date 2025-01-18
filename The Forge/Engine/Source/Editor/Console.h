#pragma once
#include "Engine/Rendering/UIWindow.h"

namespace Editor
{
    class Console final : public Engine::UIWindow
    {
    public:
        Console();
        ~Console() = default;
        
        void Render() override;

    private:
    };
}
