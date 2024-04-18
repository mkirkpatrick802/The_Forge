#pragma once

#include "EventDispatcher.h"
#include "NetCode.h"

class ByteStream;

class Client : public NetCode, public EventDispatcher
{
public:

	void Start() override;
	void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info) override;

	static void SendByteStreamToServer(const ByteStream& message);
	static bool IsHostClient() { return isHostClient; }

protected:

	void PollIncomingMessages() override;
	void ReadByteStream(const char* buffer) override;

public:

	static bool isHostClient;
	bool isConnected = false;

private:

	uint32 _playerID;
	static HSteamNetConnection _connection;

};
