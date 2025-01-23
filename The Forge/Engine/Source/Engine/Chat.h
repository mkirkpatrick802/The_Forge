#pragma once
#include <memory>

namespace Engine
{
    class ChatWindow;
    class InputManager;
    class Chat
    {
    public:

        Chat();
        Chat(const std::shared_ptr<InputManager>& inputManager);

        ~Chat();
        
        void Update(float deltaTime);

    private:

        void OpenChatWindow(bool slash);
        void CloseChatWindow();

    private:

        std::shared_ptr<InputManager> _inputManager;
        std::shared_ptr<ChatWindow> _window;
    };
}
