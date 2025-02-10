#include "Chat.h"

#include <SDL_scancode.h>

#include "ChatWindow.h"
#include "EventSystem.h"
#include "GameEngine.h"
#include "InputManager.h"
#include "Rendering/UIManager.h"

Engine::Chat::Chat()
{
    EventSystem::GetInstance()->RegisterEvent("Editor Enabled", this, &Chat::CloseChatWindow);
}

Engine::Chat::~Chat()
{
    EventSystem::GetInstance()->DeregisterEvent("Editor Enabled", this);

    CloseChatWindow();
}

void Engine::Chat::Update(float deltaTime)
{
    if (!_window)
    {
        _window = std::make_shared<ChatWindow>();
        UIManager::AddUIWindow(_window);
    }
    
    if (GetInputManager().GetKeyDown(SDL_SCANCODE_T))
        _window->EnableTerminal(true);

    if (GetInputManager().GetKeyDown(SDL_SCANCODE_SLASH))
        _window->EnableTerminal(true);
			
    if (GetInputManager().GetKeyDown(SDL_SCANCODE_ESCAPE))
        _window->EnableTerminal(false);
}

void Engine::Chat::CloseChatWindow(const void* p)
{
    if (_window == nullptr) return;

    UIManager::RemoveUIWindow(_window);
    _window.reset();
    _window = nullptr;
}
