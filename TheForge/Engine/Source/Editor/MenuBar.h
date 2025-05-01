#pragma once
#include "Engine/Rendering/UIWindow.h"

namespace Editor
{
    class MenuBar final : public Engine::UIWindow
    {
    public:
        void Render() override;
    };
}
