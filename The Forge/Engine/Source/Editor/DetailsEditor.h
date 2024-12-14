#pragma once
#include "Engine/UIWindow.h"

namespace Editor
{
    class DetailsEditor final : public Engine::UIWindow
    {
    public:
        void Render() override;
    };
}
