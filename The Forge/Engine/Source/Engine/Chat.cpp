#include "Chat.h"

#include <SDL_scancode.h>

#include "ChatWindow.h"
#include "InputManager.h"
#include "Rendering/UIManager.h"

Engine::Chat::Chat()
{
    
}

Engine::Chat::Chat(const std::shared_ptr<InputManager>& inputManager)
{
    _inputManager = inputManager;
}

Engine::Chat::~Chat()
{
    _window->CleanUp();
}

void Engine::Chat::Update(float deltaTime)
{
    if (_inputManager->GetKeyDown(SDL_SCANCODE_T))
        OpenChatWindow(false);

    if (_inputManager->GetKeyDown(SDL_SCANCODE_SLASH))
        OpenChatWindow(true);
			
    if (_inputManager->GetKeyDown(SDL_SCANCODE_ESCAPE))
        CloseChatWindow();
}

void Engine::Chat::OpenChatWindow(bool slash)
{
    if (_window != nullptr) return;
    
    _window = std::make_shared<ChatWindow>();
    UIManager::AddUIWindow(_window);
}

void Engine::Chat::CloseChatWindow()
{
    if (_window == nullptr) return;

    UIManager::RemoveUIWindow(_window);
    _window.reset();
    _window = nullptr;
}
