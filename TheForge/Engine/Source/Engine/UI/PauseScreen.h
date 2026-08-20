#pragma once
#include "UIScreen.h"

namespace Engine
{
    // Esc during play: Resume, Settings, Quit to Menu.
    //
    // Transparent on purpose. It is the one screen that sits over a live game rather
    // than replacing it, which makes it the proof that the UI layer composites over the
    // scene rather than only working on an empty one.
    //
    // Note that the game keeps running underneath. There is no pause: the world is
    // simulated on the server, and a client that stopped ticking would simply stop
    // hearing about it. "Pause" here means "a menu is open", which is the only thing it
    // can mean in a multiplayer game.
    class PauseScreen final : public UIScreen
    {
    public:
        static constexpr const char* SCREEN_NAME = "Pause";

        PauseScreen() : UIScreen(SCREEN_NAME) {}

        void OnEnter() override;

        bool IsTransparent() const override { return true; }
        bool IsInputTransparent() const override { return false; }
    };
}
