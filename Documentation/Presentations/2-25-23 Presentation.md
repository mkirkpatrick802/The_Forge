---
tags:
  - Presentations
cssclasses: []
---
# 2/25/23 Check-in
By Michael Kirkpatrick

---
## What has been done?
- Standard Game Engine Systems
- Basic Editor Setup
- Basic Replication System

---
### Standard Systems
* Component-Based Game Objects
* Renderer
* Input System
* JSON Serialization

---
### Editor Setup
![](Presentations/Images/editor_2-25-25.png)

---
## Now Netcode

---
### Lobby System
``` C++

// Set search parameters
SteamMatchmaking()->AddRequestLobbyListStringFilter("game", GAME_NAME, k_ELobbyComparisonEqual); 
SteamMatchmaking()->AddRequestLobbyListResultCountFilter(1);  
  
// Start lobby search  
const SteamAPICall_t call = SteamMatchmaking()->RequestLobbyList();  
_impl->lobbyMatchListResult.Set(call, _impl.get(), &Impl::OnLobbyMatchListCallback);

// If we have available lobbies, enter the first one  
lobbyID = SteamMatchmaking()->GetLobbyByIndex(0);  
SteamAPICall_t call = SteamMatchmaking()->JoinLobby(lobbyID);  
lobbyEnteredResult.Set( call, this, &Impl::OnLobbyEnteredCallback );

// Else we need to make our own lobby  
SteamAPICall_t call = SteamMatchmaking()->CreateLobby(k_ELobbyTypePublic, 4);  
lobbyCreateResult.Set(call, this, &Impl::OnLobbyCreateCallback);

```
---
### P2P Message System
```C++
bool GamerServices::SendP2PReliable(const OutputByteStream& inOutputStream, uint64_t inToPlayer)  
{  
    return SteamNetworking()->SendP2PPacket(inToPlayer, inOutputStream.GetBuffer(), inOutputStream.GetByteLength(), k_EP2PSendReliable);  
}  
  
bool GamerServices::IsP2PPacketAvailable(uint32_t& outPacketSize)  
{  
    return SteamNetworking()->IsP2PPacketAvailable(&outPacketSize);  
}  
  
uint32_t GamerServices::ReadP2PPacket(void* inToReceive, uint32_t inMaxLength, uint64_t& outFromPlayer)  
{  
    uint32_t packetSize;  
    CSteamID fromID;  
    SteamNetworking()->ReadP2PPacket(inToReceive, inMaxLength, &packetSize, &fromID);  
    outFromPlayer = fromID.ConvertToUint64();  
    return packetSize;  
}
```
---
### Game Object ID's
```C++
uint32_t NetCode::LinkingContext::GetNetworkID(Engine::GameObject* go, const bool shouldCreateIfNotFound)  
{  
    if (const auto it = _networkIDMap.find(go); it != _networkIDMap.end())  
        return it->second;
        
    if (shouldCreateIfNotFound)  
    {        const uint32_t networkID = _nextNetworkID++;  
        AddGameObject(go, networkID);  
        return networkID;  
    }    
    return NULL_ID;  
}

Engine::GameObject* NetCode::LinkingContext::GetGameObject(const uint32_t networkID)  
{  
    if (const auto it = _goMap.find(networkID); it != _goMap.end())  
        return it->second;  
  
    return nullptr;  
}
```
---
### Packet Processing
```C++
while(GetGamerService().IsP2PPacketAvailable(incomingSize) && receivedPackedCount < 10)  
{  
    if(incomingSize <= packetSize)  
    {        const uint32_t readByteCount = GetGamerService().ReadP2PPacket(stream.GetBuffer(), packetSize, fromPlayer);  
        if(readByteCount > 0)  
        {            stream.ResetToCapacity(readByteCount);  
            ++receivedPackedCount;  
            // Shove the packet into the queue, and we'll handle it as soon as we should...  
            _packetQueue.emplace(stream, fromPlayer);  
        }    
    }
}

while (!_packetQueue.empty())  
{  
    ReceivedPacket& nextPacket = _packetQueue.front();  
    ProcessPacket(nextPacket.GetByteStream(), nextPacket.GetFromPlayer());  
    _packetQueue.pop();  
}

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
default:  
    DEBUG_LOG("Unhandled Packet Received!")  
}
```

---
## Current Blockers
- Starting world state packet size
- Not enough "game"
- Testing

---
## Next Up
- Cut up world state
- Condense byte stream
- Collisions
- Sorting layer clean up

---
![](Presentations/Videos/)