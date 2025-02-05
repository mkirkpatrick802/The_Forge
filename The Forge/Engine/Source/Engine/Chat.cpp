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
    _window->CleanUp();
    EventSystem::GetInstance()->DeregisterEvent("Editor Enabled");
}

void Engine::Chat::Update(float deltaTime)
{
    if (GetInputManager().GetKeyDown(SDL_SCANCODE_T))
        OpenChatWindow(false);

    if (GetInputManager().GetKeyDown(SDL_SCANCODE_SLASH))
        OpenChatWindow(true);
			
    if (GetInputManager().GetKeyDown(SDL_SCANCODE_ESCAPE))
        CloseChatWindow();
}

void Engine::Chat::OpenChatWindow(bool slash)
{
    if (_window != nullptr) return;
    
    _window = std::make_shared<ChatWindow>();
    UIManager::AddUIWindow(_window);
}

void Engine::Chat::CloseChatWindow(const void* p)
{
    if (_window == nullptr) return;

    UIManager::RemoveUIWindow(_window);
    _window.reset();
    _window = nullptr;
}
