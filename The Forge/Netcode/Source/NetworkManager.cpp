#include "NetworkManager.h"
#include <memory>

#include "GamerServices.h"
#include "Engine/GameModeBase.h"
#include "Engine/Level.h"
#include "Engine/LevelManager.h"
#include "Engine/System.h"

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
    
}

void NetCode::NetworkManager::StartNetCode()
{
    // Begin the search for a lobby
    _state = NMS_Searching;
    GetGamerService().LobbySearchAsync();
}

void NetCode::NetworkManager::Update()
{
    
}

void NetCode::NetworkManager::ProcessIncomingPackets()
{
    ReadIncomingPacketsIntoQueue();
    ProcessQueuedPackets();
    UpdateBytesSentLastFrame();
}

void NetCode::NetworkManager::ReadIncomingPacketsIntoQueue()
{
    uint32_t packetSize = 1500;
    uint32_t incomingSize = 0;
    InputByteStream stream(packetSize * 8);
    uint64_t fromPlayer;

    //keep reading until we don't have anything to read ( or we hit a max number that we'll process per frame )
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

                //shove the packet into the queue and we'll handle it as soon as we should...
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
    std::string type;
    stream.Read(type);

    if (type == "Start")
    {
        Engine::LevelManager::GetCurrentLevel()->Read(stream);
    }
}

void NetCode::NetworkManager::UpdateBytesSentLastFrame()
{
    
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
    DEBUG_LOG("Onboard new player: %d", playerID)
    auto player = Engine::LevelManager::GetCurrentLevel()->GetGameMode().SpawnPlayer(playerID);
    
    if (playerID != _ownerID)
    {
        OutputByteStream stream;
        std::string type("Start");
        stream.Write(type);
        Engine::LevelManager::GetCurrentLevel()->Write(stream);
        GetGamerService().SendP2PReliable(stream, playerID);
    }
}

bool NetCode::NetworkManager::IsPlayerInGame(uint64_t playerID) const
{
    if (_playerNames.contains(playerID))
        return true;

    return false;
}

void NetCode::NetworkManager::HandleConnectionReset(uint64_t playerID)
{
    if (_playerNames.contains(playerID))
    {
        DEBUG_LOG("Player %llu disconnected", playerID)
        _playerNames.erase(playerID);
        _playerCount--;
    }
}
