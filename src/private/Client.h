#pragma once

#include "EventDispatcher.h"
#include "NetCode.h"

class ByteStream;

class Client : public NetCode, public EventDispatcher
{
public:

	virtual void Start() override;
	virtual void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info) override;

	static void SendByteStreamToServer(const ByteStream& message);
	static bool IsHostClient() { return isHostClient; }

	static uint8 GetPlayerID() { return _playerID; }

protected:

	virtual void PollIncomingMessages() override;
	virtual void ReadByteStream(const char* buffer) override;

public:

	static bool isHostClient;
	bool isConnected = false;

private:

	static uint8 _playerID;
	static HSteamNetConnection _connection;

};
