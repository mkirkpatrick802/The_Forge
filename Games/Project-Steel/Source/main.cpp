#include "Engine/EngineManager.h"
#include "Engine/GameEngine.h"
#include "Engine/LaunchOptions.h"
#include "Engine/System.h"

using namespace Engine;
using namespace Editor;

int main(int argc, char** argv)
{
    // Must run before anything touches System, GameEngine, or the launch options.
    ParseLaunchOptions(argc, argv);
    const LaunchOptions& options = GetLaunchOptions();

    // The dedicated server model uses its own transport, so there is no Steam
    // identity to establish and no reason to require a running Steam client.
    if (options.UsesDedicatedServerModel())
        REQUIRE_GAMER_SERVICES = false;

    // Created even when headless -- it is hidden in that case, but the GL context
    // it carries is what lets a level load its shaders and textures normally.
    CreateAppWindow();

#ifdef DEBUG
    // The editor is a single-player authoring tool -- never bring it up for a
    // server or a client that is about to join one.
    GetEngineManager().ToggleEditor(options.UsesDedicatedServerModel() ? "0" : "1");
#else
    GetEngineManager().ToggleEditor("0");
#endif

    GetGameEngine().StartGameplayLoop();
}
