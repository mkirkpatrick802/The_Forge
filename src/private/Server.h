#pragma once
#include <map>
#include <string>

#include "NetCode.h"

class ByteStream;
class Client;

class Server : public NetCode
{
public:

	Server();

	void Start() override;
	void Update() override;

	void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info) override;

	bool IsServerClientConnected() const;

	Client* GetClient() const { return _client; }

private:

	virtual void PollIncomingMessages() override;

	void SetClientNickname(HSteamNetConnection connection, const char* nickname);

	void SendStringToClient(HSteamNetConnection connection, const char* str) const;
	void SendStringToClient(HSteamNetConnection connection, const char* str, int size) const;
	void SendStringToAllClients(const char* str, HSteamNetConnection except = k_HSteamNetConnection_Invalid);

	void SendByteStreamToClient(HSteamNetConnection connection, const ByteStream& byteStream) const;
	void SendByteSteamToAllClients(const ByteStream& byteStream);

protected:

	virtual void ReadByteStream(const char* buffer) override;

private:

	HSteamListenSocket _listenSocket;
	HSteamNetPollGroup _pollGroup;

	struct ClientObject
	{
		std::string nickname;
	};

	std::map< HSteamNetConnection, ClientObject > _mapClients;

	Client* _client;
	HSteamNetConnection _clientWaitingForMessage;
};
