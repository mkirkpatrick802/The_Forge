#pragma once
#include "Engine/Rendering/UIWindow.h"

namespace Editor
{
    class SceneDisplay final : public Engine::UIWindow
    {
    public:

        SceneDisplay();
        void Render() override;

    private:
    };
}
