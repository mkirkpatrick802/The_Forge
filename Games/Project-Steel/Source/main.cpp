#include "Engine/EngineManager.h"
#include "Engine/GameEngine.h"
#include "Engine/System.h"

using namespace Engine;
using namespace Editor;

int main()
{
    CreateAppWindow();

#ifdef DEBUG
        GetEngineManager().ToggleEditor("1");
#else
        GetEngineManager().ToggleEditor("0");
#endif
    
        GetGameEngine().StartGameplayLoop();
}
