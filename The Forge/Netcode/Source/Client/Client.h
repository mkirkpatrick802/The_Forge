#pragma once
#include <cstdint>
#include <string>
#include <steam/steam_api_common.h>
#include <steam/isteammatchmaking.h>

#include "Utilites/NetObject.h"

namespace NetCode
{
	class Client final : public NetObject
	{
	public:
		
		Client() = default;
		Client(std::string name);
		~Client() override = default;

	private:

		STEAM_CALLBACK(Client, OnLobbyEntered, LobbyEnter_t);
		STEAM_CALLBACK(Client, OnLobbyChatUpdate, LobbyChatUpdate_t);

	private:

		void OnLobbyListFound(LobbyMatchList_t *pLobbyMatchList, bool bIOFailure);

	private:

		std::string _lobbyName;
		CSteamID _currentLobby;
		CCallResult<Client, LobbyMatchList_t > _CallResultLobbyMatchList;
		
	};
}
