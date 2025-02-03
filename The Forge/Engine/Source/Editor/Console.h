#pragma once
#include <vector>

#include "Engine/System.h"
#include "Engine/Rendering/UIWindow.h"


namespace Editor
{
    class Console final : public Engine::UIWindow
    {
    public:

        Console();
        ~Console() override;
        
        void Render() override;
        void AddToLog(const void* data);
        
    private:

        std::vector<std::pair<std::string, Engine::LogType>> _log;
        
    };
}
