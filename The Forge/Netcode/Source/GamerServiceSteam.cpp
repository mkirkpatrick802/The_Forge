#include <iostream>

#include "ByteStream.h"
#include "GamerServices.h"
#include "NetworkManager.h"
#include "Engine/System.h"
#include "steam/steam_api.h"

using namespace NetCode;
std::unique_ptr< GamerServices > GamerServices::instance = nullptr;

//TODO: This should be based off the build project or smth
static auto GAME_NAME = "Project_Steel";

struct GamerServices::Impl
{
    Impl();

    // Call result when we get a list of lobbies
    CCallResult<Impl, LobbyMatchList_t> lobbyMatchListResult;
    void OnLobbyMatchListCallback( LobbyMatchList_t* inCallback, bool inIOFailure );
    
    // Call result when a lobby is created
    CCallResult<Impl, LobbyCreated_t> lobbyCreateResult;
    void OnLobbyCreateCallback( LobbyCreated_t* inCallback, bool inIOFailure );
    
    // Call result when we enter a lobby
    CCallResult<Impl, LobbyEnter_t> lobbyEnteredResult;
    void OnLobbyEnteredCallback( LobbyEnter_t* inCallback, bool inIOFailure );

    // Callback when a user leaves/enters lobby
    STEAM_CALLBACK( Impl, OnLobbyChatUpdate, LobbyChatUpdate_t, mChatDataUpdateCallback );
    
    //Callback when a P2P connection is attempted
    STEAM_CALLBACK( Impl, OnP2PSessionRequest, P2PSessionRequest_t, mSessionRequestCallback );
    
    // Callback if the P2P connection fails
    STEAM_CALLBACK( Impl, OnP2PSessionFail, P2PSessionConnectFail_t, mSessionFailCallback );

    uint64_t gameID;
    CSteamID lobbyID;
};

GamerServices::Impl::Impl():
    mChatDataUpdateCallback(this, &Impl::OnLobbyChatUpdate),
    mSessionRequestCallback(this, &Impl::OnP2PSessionRequest),
    mSessionFailCallback(this, &Impl::OnP2PSessionFail),
    gameID(SteamUtils()->GetAppID())
{
}

void GamerServices::Impl::OnLobbyMatchListCallback(LobbyMatchList_t* inCallback, bool inIOFailure)
{
    if (inIOFailure) return;
    
    if(inCallback->m_nLobbiesMatching > 0)
    {
        DEBUG_LOG("Lobby found!");

        // If we have available lobbies, enter the first one
        lobbyID = SteamMatchmaking()->GetLobbyByIndex(0);
        SteamAPICall_t call = SteamMatchmaking()->JoinLobby(lobbyID);
        lobbyEnteredResult.Set( call, this, &Impl::OnLobbyEnteredCallback );
    }
    else
    {
        DEBUG_LOG("Not lobby found, creating one...")
        
        // Need to make our own lobby
        SteamAPICall_t call = SteamMatchmaking()->CreateLobby(k_ELobbyTypePublic, 4);
        lobbyCreateResult.Set(call, this, &Impl::OnLobbyCreateCallback);
    }
}

void GamerServices::Impl::OnLobbyCreateCallback(LobbyCreated_t* inCallback, bool inIOFailure)
{
    if(inCallback->m_eResult == k_EResultOK && !inIOFailure)
    {
        lobbyID = inCallback->m_ulSteamIDLobby;
        DEBUG_LOG("Lobby created!")
        
        // Set our game so others can find this lobby
        SteamMatchmaking()->SetLobbyData(lobbyID, "game", GAME_NAME);
        GetNetworkManager().EnterLobby(lobbyID.ConvertToUint64());
    }
}

void GamerServices::Impl::OnLobbyEnteredCallback(LobbyEnter_t* inCallback, bool inIOFailure)
{
    if(inIOFailure) return;

    if (inCallback->m_EChatRoomEnterResponse == k_EChatRoomEnterResponseSuccess)
    {
        lobbyID = inCallback->m_ulSteamIDLobby;
        DEBUG_LOG("Lobby joined!");
        DEBUG_LOG("Lobby host: %d", GetGamerService().GetOwnerID(lobbyID.ConvertToUint64()))
        
        GetNetworkManager().EnterLobby(lobbyID.ConvertToUint64());
    }
    else
    {
        DEBUG_LOG("Failed to join lobby!")
    }
}

void GamerServices::Impl::OnLobbyChatUpdate(LobbyChatUpdate_t* inCallback)
{
    GetNetworkManager().UpdateLobbyPlayers();

    if (inCallback->m_rgfChatMemberStateChange & k_EChatMemberStateChangeEntered)
    {
        // New player has joined the game
        if (!GetNetworkManager().GetIsOwner()) return;
        GetNetworkManager().OnboardNewPlayer(inCallback->m_ulSteamIDUserChanged);
    }
}

void GamerServices::Impl::OnP2PSessionRequest( P2PSessionRequest_t* inCallback )
{
    CSteamID playerID = inCallback->m_steamIDRemote;
    if (GetNetworkManager().IsPlayerInGame(playerID.ConvertToUint64()))
        SteamNetworking()->AcceptP2PSessionWithUser(playerID);
}

void GamerServices::Impl::OnP2PSessionFail( P2PSessionConnectFail_t* inCallback )
{
    GetNetworkManager().HandleConnectionReset(inCallback->m_steamIDRemote.ConvertToUint64());
}
/*
 *      Gamer Services Exclusive
 */

void GamerServices::Init()
{
    if(!SteamAPI_Init())
    {
        std::cout << "Failed to initialize Steam. Make sure you are running a Steam client." << '\n';
        MessageBox(NULL, L"Failed to initialize Steam. Make sure you are running a Steam client.", L"Error", MB_OK | MB_ICONERROR);
        exit(1);
    }
    else
    {
        // Can't use std::make_unique because we want to use a private constructor
        instance.reset(new GamerServices());
    }
}

void GamerServices::CleanUp()
{
    instance.reset();
    instance = nullptr;
}

GamerServices::GamerServices()
{
    _impl = std::make_unique<Impl>();
}

void GamerServices::LobbySearchAsync()
{
    DEBUG_LOG("Searching for lobby...");
    
    // Set search parameters
    SteamMatchmaking()->AddRequestLobbyListStringFilter("game", GAME_NAME, k_ELobbyComparisonEqual);
    SteamMatchmaking()->AddRequestLobbyListResultCountFilter(1);

    // Start lobby search
    const SteamAPICall_t call = SteamMatchmaking()->RequestLobbyList();
    _impl->lobbyMatchListResult.Set(call, _impl.get(), &Impl::OnLobbyMatchListCallback);
}

void GamerServices::Update() const
{
    //without this, callbacks will never fire
    SteamAPI_RunCallbacks();
}

uint64_t GamerServices::GetLocalPlayerID() const
{
    const CSteamID id = SteamUser()->GetSteamID();
    return id.ConvertToUint64();
}

uint64_t GamerServices::GetOwnerID(const uint64_t inLobbyID) const
{
    return SteamMatchmaking()->GetLobbyOwner(inLobbyID).ConvertToUint64();
}

void GamerServices::GetLobbyPlayerMap(const uint64_t inLobbyID, std::map<uint64_t, std::string>& outPlayerMap) const
{
    const CSteamID localID = SteamUser()->GetSteamID();
    outPlayerMap.clear();

    const int count = GetLobbyNumPlayers(inLobbyID);
    for (int i = 0; i < count; i++)
    {
        CSteamID playerID = SteamMatchmaking()->GetLobbyMemberByIndex(inLobbyID, i);
        if (playerID == localID)
        {
            outPlayerMap.emplace(playerID.ConvertToUint64(), GetLocalPlayerName());
        }
        else
        {
            outPlayerMap.emplace(playerID.ConvertToUint64(), GetRemotePlayerName(playerID.ConvertToUint64()));
        }
    }
}

void GamerServices::LeaveLobby(uint64_t inLobbyID)
{
    
}

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

int GamerServices::GetLobbyNumPlayers(const uint64_t inLobbyID) const
{
    return SteamMatchmaking()->GetNumLobbyMembers(inLobbyID);
}

std::string GamerServices::GetLocalPlayerName() const
{
    return SteamFriends()->GetPersonaName();
}

std::string GamerServices::GetRemotePlayerName(const uint64_t inPlayerID) const
{
    return SteamFriends()->GetFriendPersonaName(inPlayerID);
}

GamerServices::~GamerServices()
{
    SteamAPI_Shutdown();
}
