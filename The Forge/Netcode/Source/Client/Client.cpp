#include "Client.h"

#include <iostream>
#include <steam/isteammatchmaking.h>
#include <steam/steamclientpublic.h>

NetCode::Client::Client(std::string name)
{
    // Request the lobby list
    SteamMatchmaking()->AddRequestLobbyListStringFilter("name", name.c_str(), k_ELobbyComparisonEqual);
    SteamAPICall_t hSteamAPICall = SteamMatchmaking()->RequestLobbyList();
    _CallResultLobbyMatchList.Set( hSteamAPICall, this, &Client::OnLobbyListFound);

    _lobbyName = name;
}

void NetCode::Client::OnLobbyListFound(LobbyMatchList_t* pLobbyMatchList, bool bIOFailure)
{
    // Loop through all the lobbies found in the list
    for (int i = 0; i < pLobbyMatchList->m_nLobbiesMatching; ++i)
    {
        CSteamID lobbyID = SteamMatchmaking()->GetLobbyByIndex(i);
        
        // Get the name of the lobby
        const char* currentLobbyName = SteamMatchmaking()->GetLobbyData(lobbyID, "name");
        if (currentLobbyName && _lobbyName == currentLobbyName)
        {
            // Lobby found, join it
            SteamMatchmaking()->JoinLobby(lobbyID);
            std::cout << "Joining Lobby: " << _lobbyName << " (" << lobbyID.ConvertToUint64() << ")\n";
            return;
        }
    }

    // If no matching lobby was found
    std::cerr << "Lobby with name '" << _lobbyName << "' not found.\n";
}

void NetCode::Client::OnLobbyEntered(LobbyEnter_t* pParam)
{
    if (pParam->m_EChatRoomEnterResponse == k_EChatRoomEnterResponseSuccess)
    {
        _currentLobby = pParam->m_ulSteamIDLobby;
        std::cout << "Entered Lobby: " << _currentLobby.ConvertToUint64() << '\n';
    }
    else
    {
        std::cerr << "Failed to enter lobby!" << '\n';
    }
}

void NetCode::Client::OnLobbyChatUpdate(LobbyChatUpdate_t* pParam)
{
    CSteamID userChanged(pParam->m_ulSteamIDUserChanged);

    // Fetch the username of the user who triggered the update
    const char* username = SteamFriends()->GetFriendPersonaName(userChanged);
    
    // Check if a user joined the lobby
    if (pParam->m_rgfChatMemberStateChange & k_EChatMemberStateChangeEntered) {
        std::cout << "User joined the lobby: " << username << '\n';
    }

    // Check if a user left the lobby
    if (pParam->m_rgfChatMemberStateChange & k_EChatMemberStateChangeLeft) {
        std::cout << "User left the lobby: " << username << '\n';
    }

    // Check if a user was disconnected
    if (pParam->m_rgfChatMemberStateChange & k_EChatMemberStateChangeDisconnected) {
        std::cout << "User disconnected from the lobby: " << username << '\n';
    }

    // Check if a user was kicked
    if (pParam->m_rgfChatMemberStateChange & k_EChatMemberStateChangeKicked) {
        std::cout << "User was kicked from the lobby: " << username << '\n';
    }

    // Check if a user was banned
    if (pParam->m_rgfChatMemberStateChange & k_EChatMemberStateChangeBanned) {
        std::cout << "User was banned from the lobby: " << username << '\n';
    }
}