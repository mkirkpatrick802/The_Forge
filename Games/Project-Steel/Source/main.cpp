#include "Engine/EngineManager.h"
#include "Engine/GameEngine.h"
#include "Engine/System.h"

using namespace Engine;
using namespace Editor;

int main()
{
    CreateAppWindow();
    GetEngineManager().ToggleEditor("1");
    GetGameEngine().StartGameplayLoop();
}
