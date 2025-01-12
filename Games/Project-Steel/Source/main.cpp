#include "Engine/EngineManager.h"
#include "Engine/EventSystem.h"
#include "Engine/GameEngine.h"
#include "Engine/System.h"

using namespace Engine;
using namespace Editor;

int main()
{
    System::Init();

    {
        std::unique_ptr<EngineManager> engine = std::make_unique<EngineManager>();
        
        System::CreateAppWindow(Vector2D(800, 600));
        
        auto instance = GameEngine::GetInstance();
        
        instance->StartGamePlayLoop();
        instance->CleanUp();
        delete instance;
        instance = nullptr;
    }
    
    System::CleanUp();
}
