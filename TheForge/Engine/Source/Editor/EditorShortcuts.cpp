#include "EditorShortcuts.h"
#include "Engine/InputManager.h"

#include <memory>
#include <SDL_scancode.h>

#include "Engine/GameEngine.h"
#include "Engine/Level.h"
#include "Engine/LevelManager.h"
#include "Engine/System.h"

Editor::EditorShortcuts& Editor::EditorShortcuts::GetInstance()
{
    static auto instance = std::make_unique<EditorShortcuts>();
    return *instance;
}

void Editor::EditorShortcuts::Update()
{
    if (Engine::GetInputManager().GetKeyDown(SDL_SCANCODE_S) && Engine::GetInputManager().GetKey(SDL_SCANCODE_LCTRL))
    {
        Engine::GetLevelManager().GetCurrentLevel()->SaveLevel();
    }
}
