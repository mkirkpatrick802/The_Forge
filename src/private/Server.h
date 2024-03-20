#pragma once
#include <map>
#include <string>

#include "NetCode.h"

class Client;

class Server : public NetCode
{
public:

	Server();

	void Start() override;
	void Update() override;

	void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info) override;

private:

	void PollIncomingMessages() override;
	void PollLocalUserInput();

	void SetClientNickname(HSteamNetConnection connection, const char* nickname);
	void SendStringToClient(HSteamNetConnection connection, const char* str);
	void SendStringToAllClients(const char* str, HSteamNetConnection except = k_HSteamNetConnection_Invalid);

private:

	HSteamListenSocket _listenSocket;
	HSteamNetPollGroup _pollGroup;

	struct ClientObject
	{
		std::string nickname;
	};

	std::map< HSteamNetConnection, ClientObject > _mapClients;

	Client* _client;
};
