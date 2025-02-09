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
        ~Chat();
        
        void Update(float deltaTime);
        
        void CloseChatWindow(const void* p = nullptr);

    private:
        
        std::shared_ptr<ChatWindow> _window;
    };
}
