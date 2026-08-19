#include "Engine/EngineManager.h"
#include "Engine/GameEngine.h"
#include "Engine/LaunchOptions.h"
#include "Engine/System.h"
#include "NetworkManager.h"

using namespace Engine;
using namespace Editor;

int main(int argc, char** argv)
{
    // Must run before anything touches System, GameEngine, or the launch options.
    ParseLaunchOptions(argc, argv);
    const LaunchOptions& options = GetLaunchOptions();

    // The dedicated server model uses its own transport, and --no-steam covers
    // editor/offline work -- neither has a reason to need a running Steam client.
    REQUIRE_GAMER_SERVICES = options.RequiresSteam();

    // No window, and therefore no GL context, when headless. Level loading creates
    // no GPU resources, so a dedicated server needs neither.
    if (!options.headless)
        CreateAppWindow();

#ifdef DEBUG
    // The editor is a single-player authoring tool -- never bring it up for a
    // server or a client that is about to join one.
    GetEngineManager().ToggleEditor(options.UsesDedicatedServerModel() ? "0" : "1");
#else
    GetEngineManager().ToggleEditor("0");
#endif

    // The transport is chosen and started by the NetworkManager -- Steam, UDP or
    // none, depending on the role parsed above. It comes up in StartNetCode() once
    // the level exists, so a connection can never be accepted with nothing to
    // spawn into.
    GetGameEngine().StartGameplayLoop();

    NetCode::GetNetworkManager().ShutdownNetCode();
}
