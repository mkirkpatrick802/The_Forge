#include "NetworkManager.h"
#include <memory>

#include "GamerServices.h"
#include "LinkingContext.h"
#include "Engine/GameModeBase.h"
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

NetCode::NetworkManager::NetworkManager(): _state(NMS_Unitialized), _lobbyID(0), _ownerID(0), _playerCount(0),
                                           _isOwner(false)
{
    _localUserID = GetGamerService().GetLocalPlayerID();
}

NetCode::NetworkManager::~NetworkManager()
{
    GetGamerService().LeaveLobby(_lobbyID);
}

void NetCode::NetworkManager::StartNetCode()
{
    // Begin the search for a lobby
    _state = NMS_Searching;
    GetGamerService().LobbySearchAsync();
}

void NetCode::NetworkManager::Update()
{
    ProcessIncomingPackets();

    SendWorldStateUpdate();
}

void NetCode::NetworkManager::ProcessIncomingPackets()
{
    ReadIncomingPacketsIntoQueue();
    ProcessQueuedPackets();
}

void NetCode::NetworkManager::ReadIncomingPacketsIntoQueue()
{
    uint32_t packetSize = 1500;
    uint32_t incomingSize = 0;
    InputByteStream stream(packetSize * 8);
    uint64_t fromPlayer;

    // Keep reading until we don't have anything to read (or we hit a max number that we'll process per frame)
    int receivedPackedCount = 0;
    while(GetGamerService().IsP2PPacketAvailable(incomingSize) && receivedPackedCount < 10)
    {
        if(incomingSize <= packetSize)
        {
            const uint32_t readByteCount = GetGamerService().ReadP2PPacket(stream.GetBuffer(), packetSize, fromPlayer);
            if(readByteCount > 0)
            {
                stream.ResetToCapacity(readByteCount);
                ++receivedPackedCount;

                // Shove the packet into the queue, and we'll handle it as soon as we should...
                _packetQueue.emplace(stream, fromPlayer);
            }
        }
    }
}

void NetCode::NetworkManager::ProcessQueuedPackets()
{
    while (!_packetQueue.empty())
    {
        ReceivedPacket& nextPacket = _packetQueue.front();
        ProcessPacket(nextPacket.GetByteStream(), nextPacket.GetFromPlayer());
        _packetQueue.pop();
    }
}

void NetCode::NetworkManager::ProcessPacket(InputByteStream& stream, uint64_t playerID)
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
        _readyToGetUpdates = true;
        break;
    case PT_WorldStateUpdate:
        if (!_readyToGetUpdates) break;
        Engine::LevelManager::GetCurrentLevel()->Read(stream);
        break;
    case PT_Disconnect:
        break;
    default:
        DEBUG_LOG("Unhandled Packet Received!")
    }
}

void NetCode::NetworkManager::SendWorldStateUpdate()
{
    if (_playerCount <= 1) return;

    const uint64_t currentTicks = Engine::Time::GetTicks();
    if (currentTicks - _lastUpdateSentTicks >= _targetStateUpdateDelay)
    {
        _lastUpdateSentTicks = currentTicks;

        // Send Update
        OutputByteStream stream;
        stream.Write(PT_WorldStateUpdate);
        Engine::LevelManager::GetCurrentLevel()->Write(stream, false);
        for (const auto key : _playerNames | std::views::keys)
        {
            if (key == _localUserID) continue;
            GetGamerService().SendP2PReliable(stream, key);
        }
    }
}

void NetCode::NetworkManager::EnterLobby(const uint64_t lobbyID)
{
    _lobbyID = lobbyID;
    _state = NMS_Starting;
    UpdateLobbyPlayers();

    if (_isOwner)
        OnboardNewPlayer(_localUserID);
}

void NetCode::NetworkManager::UpdateLobbyPlayers()
{
    _playerCount = GetGamerService().GetLobbyNumPlayers(_lobbyID);
    _ownerID = GetGamerService().GetOwnerID(_lobbyID);
        
    // Am I the owner now?
    if( _ownerID == _localUserID )
        _isOwner = true;

    GetGamerService().GetLobbyPlayerMap(_lobbyID, _playerNames);
    DEBUG_LOG("Current player count: %d", _playerCount)
}

// TODO: Move this to a server class (only gets called on the owners machine)
// Spawn new player, and send current world state
void NetCode::NetworkManager::OnboardNewPlayer(uint64_t playerID)
{
    auto player = Engine::LevelManager::GetCurrentLevel()->GetGameMode().SpawnPlayer(playerID);
    _readyToGetUpdates = GetIsOwner(); 
    
    if (playerID != _ownerID)
    {
        // Send Welcome Message
        OutputByteStream stream;
        stream.Write(PT_Hello);
        GetGamerService().SendP2PReliable(stream, playerID);

        // Send Current World State
        OutputByteStream newStream;
        newStream.Write(PT_WorldState);
        Engine::LevelManager::GetCurrentLevel()->Write(newStream);
        GetGamerService().SendP2PReliable(newStream, playerID);
    }
}

bool NetCode::NetworkManager::IsPlayerInGame(uint64_t playerID) const
{
    if (_playerNames.contains(playerID))
        return true;

    return false;
}

// TODO: Should not use the player controller class
void NetCode::NetworkManager::HandleConnectionReset(uint64_t playerID)
{
    if (_playerNames.contains(playerID))
    {
        for (const auto players = Engine::GetComponentManager().GetPool<Engine::PlayerController>()->GetActive(); const auto player : players)
            if (player->GetControllingPlayerID() == playerID)
                Engine::LevelManager::GetCurrentLevel()->RemoveGameObject(player->gameObject);
        
        DEBUG_LOG("Player %llu disconnected", playerID)
        _playerNames.erase(playerID);
        _playerCount--;
    }
}
