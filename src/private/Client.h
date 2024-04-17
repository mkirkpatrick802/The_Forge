#pragma once
#include "NetCode.h"
#include "PlayerController.h"

class Client : public NetCode
{
public:

	friend PlayerController;

	void Start() override;
	void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info) override;

protected:

	void PollIncomingMessages() override;

public:

	bool isHostClient = false;
	bool isConnected = false;

private:

	uint32 _playerID;
	HSteamNetConnection _connection;

};
