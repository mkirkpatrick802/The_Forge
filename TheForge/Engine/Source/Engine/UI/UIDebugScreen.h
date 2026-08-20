#pragma once
#include "UIScreen.h"

namespace Engine
{
    // A test card for the UI layer, opened with "/ui".
    //
    // Deliberately checks the three things that are easy to get wrong and hard to notice:
    // that UI draws at all with no camera and no level (the whole reason this layer
    // exists), that the anchors resolve against the canvas rather than against fixed
    // coordinates, and that the canvas tracks the scene when the window is resized.
    // Markers sit in all four corners and dead centre, so any of those failing is
    // visible at a glance rather than being a plausible-looking layout.
    class UIDebugScreen final : public UIScreen
    {
    public:
        UIDebugScreen() : UIScreen("UI Debug") {}

        void OnEnter() override;

        // Transparent, so the test card can be opened over a running game to check that
        // the UI composites over the scene rather than replacing it.
        bool IsTransparent() const override { return true; }
        bool IsInputTransparent() const override { return true; }

        // Registers the "/ui" command. Called once at engine startup.
        static void RegisterCommand();
    };
}
