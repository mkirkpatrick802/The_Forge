#pragma once
#include <map>
#include <string>

#include "NetCode.h"
#include <GameMode.h>

#include "EventListener.h"

const int MAX_MESSAGES = 4;
const int MAX_NICKNAME_SIZE = 15;

class ByteStream;
class Client;

struct ClientObject
{
	uint8 playerID;
	std::string nickname;
};

class Server : public NetCode, public EventListener
{
	friend class ObjectStateWriter;

public:

	Server();

	void Start() override;
	void Update(float deltaTime) override;

	void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info) override;

	Client* GetClient() const { return _client; }

private:

	static Server* GetServer() { return _instance; }

	virtual void PollIncomingMessages() override;
	void HandleClientMessage(const std::pair<HSteamNetConnection, ClientObject>& client);
	virtual void ReadByteStream(HSteamNetConnection messageAuthor, const char* buffer) override;

	void SetClientObject(HSteamNetConnection connection, const ClientObject& object);

	void SendStringToClient(HSteamNetConnection connection, const char* str) const;
	void SendStringToClient(HSteamNetConnection connection, const char* str, int size) const;
	void SendStringToAllClients(const char* str, HSteamNetConnection except = k_HSteamNetConnection_Invalid) const;

	void SendByteStreamToClient(HSteamNetConnection connection, const ByteStream& byteStream) const;
	void SendByteSteamToAllClients(const ByteStream& byteStream, HSteamNetConnection except = k_HSteamNetConnection_Invalid) const;

	virtual void OnEvent(Event* event) override;

private:

	static Server* _instance;

	HSteamListenSocket _listenSocket;
	HSteamNetPollGroup _pollGroup;

	std::map< HSteamNetConnection, ClientObject > _mapClients;

	Client* _client;
	GameMode _gameMode;

	bool _gameStarted = false;
};
