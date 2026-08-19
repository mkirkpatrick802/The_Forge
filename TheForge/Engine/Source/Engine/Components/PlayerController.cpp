#include "PlayerController.h"

#include "NetworkManager.h"
#include "Engine/LaunchOptions.h"
#include "Engine/Rendering/CameraManager.h"

void Engine::PlayerController::Start()
{
    if (!GetCameraManager().GetActiveCamera())
        if (const auto camera = gameObject->GetComponent<Camera>(); camera && IsLocalPlayer())
            GetCameraManager().SetActiveCamera(camera);
}

void Engine::PlayerController::Update(float deltaTime)
{
    
}

void Engine::PlayerController::Write(NetCode::OutputByteStream& stream) const
{
    Component::Write(stream);

    stream.Write(_controllingPlayer);
}

void Engine::PlayerController::Read(NetCode::InputByteStream& stream)
{
    Component::Read(stream);

    stream.Read(_controllingPlayer);
}

bool Engine::PlayerController::IsLocalPlayer() const
{
    // A dedicated server has no local player, so nothing it simulates is ever
    // "local". Without this, an unassigned pawn (_controllingPlayer == 0) would
    // match the server's own placeholder id and start collecting local input.
    if (GetLaunchOptions().IsDedicatedServer()) return false;

    return _controllingPlayer == NetCode::GetNetworkManager().GetLocalUserID();
}
