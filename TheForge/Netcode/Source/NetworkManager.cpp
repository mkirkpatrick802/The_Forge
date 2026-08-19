#include "NetworkManager.h"

#include <memory>

#include "LinkingContext.h"
#include "Transport/OfflineTransport.h"
#include "Transport/SteamTransport.h"
#include "Transport/UdpNetTransport.h"
#include "Engine/GameModeBase.h"
#include "Engine/LaunchOptions.h"
#include "Engine/Level.h"
#include "Engine/LevelManager.h"
#include "Engine/System.h"
#include "Engine/Time.h"
#include "Engine/Components/ComponentManager.h"
#include "Engine/Components/PlayerController.h"

NetCode::NetworkManager& NetCode::NetworkManager::GetInstance()
{
    static auto instance = std::make_unique<NetworkManager>();
    return *instance;
}

NetCode::NetworkManager::NetworkManager()
{
    // One decision, made once: which transport this process is going to use. Every
    // "am I a server", "is Steam up", "who am I" question downstream resolves
    // through the object chosen here rather than being re-derived from the role.
    const Engine::LaunchOptions& options = Engine::GetLaunchOptions();

    if (options.UsesDedicatedServerModel())
        _transport = std::make_unique<UdpNetTransport>();
    else if (Engine::REQUIRE_GAMER_SERVICES)
        _transport = std::make_unique<SteamTransport>();
    else
        _transport = std::make_unique<OfflineTransport>();
}

NetCode::NetworkManager::~NetworkManager()
{
    ShutdownNetCode();
}

void NetCode::NetworkManager::ShutdownNetCode()
{
    // Guarded because main() shuts down explicitly -- so the goodbye packets go out
    // while the world is still standing -- and the destructor still has to cover
    // any path that does not get that far.
    if (_shutdown) return;
    _shutdown = true;
    _transport->Shutdown();
}

void NetCode::NetworkManager::StartNetCode()
{
    _state = NMS_Searching;

    if (!_transport->Start())
    {
        DEBUG_LOG("Netcode: transport failed to start.")
        return;
    }

    // The authority is playing the moment its level is up -- it has nobody to wait
    // for. A client stays in NMS_Starting until the world state arrives.
    _state = HasWorldAuthority() ? NMS_Playing : NMS_Starting;

    // With no session there is nobody to announce us, so the local player would
    // never be spawned and the camera would have nothing to follow. This is what
    // makes an offline or editor-launched game come up with a player at all.
    if (!_transport->IsSessionActive() && HasWorldAuthority())
        OnboardNewPlayer(_transport->GetLocalPeerID());
}

void NetCode::NetworkManager::Update()
{
    _transport->Update();

    ProcessConnectionEvents();
    ProcessIncomingPackets();
    SendWorldStateUpdate();
    SendClientInput();
}

void NetCode::NetworkManager::ProcessConnectionEvents()
{
    for (const PeerID peer : _transport->TakeConnectedPeers())
        OnboardNewPlayer(peer);

    for (const PeerID peer : _transport->TakeDisconnectedPeers())
        HandlePeerDisconnected(peer);
}

Engine::PlayerController* NetCode::NetworkManager::FindPlayerController(const PeerID peer) const
{
    const std::vector<Engine::Component*> components = Engine::GetComponentManager().GetAllDerivedComponents<Engine::PlayerController>();
    for (const auto component : components)
    {
        if (const auto player = dynamic_cast<Engine::PlayerController*>(component))
        {
            if (player->GetControllingPlayerID() == peer)
                return player;
        }
    }

    return nullptr;
}

void NetCode::NetworkManager::ProcessIncomingPackets()
{
    // Whole messages, in order, already reassembled -- the transport does not hand
    // up anything partial, so there is no framing left to unpick here.
    for (NetMessage& message : _transport->Receive())
        ProcessPacket(message.stream, message.from);
}

void NetCode::NetworkManager::ProcessPacket(InputByteStream& stream, const PeerID peer)
{
    PacketType type;
    stream.Read(type);
    switch (type)
    {
    case PT_Hello:
        DEBUG_LOG("Host has welcomed us into the server!")
        break;
    case PT_WorldState:
        Engine::LevelManager::LoadLevel(stream);
        _state = NMS_Playing;
        break;
    case PT_WorldStateUpdate:
        if (_state != NMS_Playing) return;
        Engine::LevelManager::GetCurrentLevel()->Read(stream);
        break;
    case PT_ClientInput:
        ApplyClientInput(stream, peer);
        break;
    default:
        DEBUG_LOG("Unhandled Packet Received!")
    }
}

void NetCode::NetworkManager::SendWorldStateUpdate()
{
    // World state is authoritative -- only the machine simulating it broadcasts.
    if (!HasWorldAuthority()) return;
    if (_state != NMS_Playing) return;

    const std::vector<PeerID> peers = _transport->GetRemotePeers();
    if (peers.empty()) return;

    const uint64_t currentTicks = Engine::Time::GetTicks();
    if (currentTicks - _lastUpdateSentTicks < _targetStateUpdateDelayMs) return;
    _lastUpdateSentTicks = currentTicks;

    OutputByteStream stream;
    stream.Write(PT_WorldStateUpdate);
    Engine::LevelManager::GetCurrentLevel()->Write(stream, false);

    for (const PeerID peer : peers)
        _transport->SendTo(peer, stream, true);
}

void NetCode::NetworkManager::SendClientInput()
{
    // The authority simulates its own input directly -- only remote clients send it.
    if (HasWorldAuthority()) return;
    if (_state != NMS_Playing) return;

    const uint64_t currentTicks = Engine::Time::GetTicks();
    if (currentTicks - _lastInputSentTicks < _targetInputSendDelayMs) return;
    _lastInputSentTicks = currentTicks;

    const auto localPlayer = FindPlayerController(_transport->GetLocalPeerID());
    if (localPlayer == nullptr) return;

    OutputByteStream stream;
    stream.Write(PT_ClientInput);
    localPlayer->WriteInput(stream);

    // Unreliable on purpose. Input is superseded every tick -- ReadInput just
    // overwrites the last values -- so a dropped packet costs one frame of stale
    // input, where reliable-ordered delivery costs a retransmit and stalls every
    // input behind it. Sent reliably at 60Hz it also exhausted the send window
    // outright once a second client joined.
    //
    // World state cannot do this: Write(stream, false) is a *delta* that clears
    // the dirty flags, so a lost update is lost for good. That is why the two go
    // out on different channels.
    _transport->SendToAuthority(stream, false);
}

void NetCode::NetworkManager::ApplyClientInput(InputByteStream& stream, const PeerID peer)
{
    // Only the authority consumes client input.
    if (!HasWorldAuthority()) return;
    if (_state != NMS_Playing) return;

    // The pawn is resolved from the *sender's* peer id rather than anything in the
    // payload, so a client can only ever drive the player it actually controls.
    if (const auto player = FindPlayerController(peer))
        player->ReadInput(stream);
}

// Spawn a player for the peer, and bring it up to date with the world.
void NetCode::NetworkManager::OnboardNewPlayer(const PeerID peer)
{
    if (!HasWorldAuthority()) return;

    if (Engine::LevelManager::GetCurrentLevel() == nullptr)
    {
        DEBUG_LOG("Netcode: peer %llu arrived before a level existed; ignoring.", peer)
        return;
    }

    Engine::LevelManager::GetCurrentLevel()->GetGameMode().SpawnPlayer(peer);
    _state = NMS_Playing;

    // A dedicated server is not a peer of its own, so it never takes this branch
    // for itself -- which is exactly why it ends up with no pawn of its own.
    if (peer == _transport->GetLocalPeerID()) return;

    OutputByteStream hello;
    hello.Write(PT_Hello);
    _transport->SendTo(peer, hello, true);

    OutputByteStream worldState;
    worldState.Write(PT_WorldState);
    Engine::LevelManager::GetCurrentLevel()->Write(worldState);
    _transport->SendTo(peer, worldState, true);
}

// TODO: Should not use the player controller class
void NetCode::NetworkManager::HandlePeerDisconnected(const PeerID peer)
{
    if (const auto player = FindPlayerController(peer))
        Engine::LevelManager::GetCurrentLevel()->RemoveGameObject(player->gameObject);

    DEBUG_LOG("Player %llu disconnected", peer)
}
