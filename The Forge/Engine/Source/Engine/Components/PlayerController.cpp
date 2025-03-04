#include "PlayerController.h"

#include "NetworkManager.h"
#include "Engine/Rendering/CameraManager.h"

void Engine::PlayerController::Update(float deltaTime)
{
    if (!GetCameraManager().GetActiveCamera())
        if (const auto camera = gameObject->GetComponent<Camera>(); camera && IsLocalPlayer())
            GetCameraManager().SetActiveCamera(camera);
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
    return _controllingPlayer == NetCode::GetNetworkManager().GetLocalUserID();
}
