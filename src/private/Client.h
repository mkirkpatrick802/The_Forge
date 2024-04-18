#pragma once

#include "EventDispatcher.h"
#include "NetCode.h"
#include "PlayerController.h"

class Client : public NetCode, public EventDispatcher
{
public:

	friend PlayerController;

	void Start() override;
	void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info) override;

protected:

	void PollIncomingMessages() override;
	void ReadByteStream(const char* buffer) override;

public:

	bool isHostClient = false;
	bool isConnected = false;

private:

	uint32 _playerID;
	HSteamNetConnection _connection;

};
