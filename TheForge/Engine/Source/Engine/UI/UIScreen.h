#pragma once
#include <string>

#include "UICanvas.h"

namespace Engine
{
    // One full-screen piece of UI with a lifetime: the main menu, the loading screen, a
    // pause menu, a settings page.
    //
    // Screens live on UIRoot's stack, so "open the settings page over the pause menu" is
    // a push and "go back" is a pop, rather than a pile of visibility flags that have to
    // be kept consistent by hand.
    class UIScreen
    {
    public:
        explicit UIScreen(std::string name) : _name(std::move(name)) {}
        virtual ~UIScreen() = default;

        UIScreen(const UIScreen&) = delete;
        UIScreen& operator=(const UIScreen&) = delete;

        // Called once when the screen is pushed. Build the canvas here, not in the
        // constructor -- a screen may be constructed well before it is shown, and
        // building on entry is what makes re-entering one cheap to reason about.
        virtual void OnEnter() {}

        // Called once when the screen is popped, before it is destroyed. Anything the
        // screen installed globally -- a log mirror, an input grab -- comes back off
        // here.
        virtual void OnExit() {}

        virtual void Update(const float deltaTime) {}

        // Whether screens below this one keep drawing. False for a main menu, which
        // covers everything; true for a pause menu, which should show the frozen game
        // behind it.
        virtual bool IsTransparent() const { return false; }

        // Whether screens below this one keep receiving input. Almost always false --
        // a menu over a menu should not let clicks through to the one underneath.
        virtual bool IsInputTransparent() const { return false; }

        UICanvas& Canvas() { return _canvas; }
        const UICanvas& Canvas() const { return _canvas; }
        const std::string& GetName() const { return _name; }

    protected:
        UICanvas _canvas;
        std::string _name;
    };
}
