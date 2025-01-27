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
        const auto engine = EngineManager::GetInstance();
        engine->ToggleEditor("1");
        
        System::CreateAppWindow();
        
        auto instance = GameEngine::GetInstance();
        
        instance->StartGamePlayLoop();
        instance->CleanUp();
        
        delete instance;
        instance = nullptr;

        EngineManager::CleanUp();
    }
    
    System::CleanUp();
}
