#pragma once
#include "NetCode.h"

class Client : public NetCode
{
public:

	void Start() override;
	void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info) override;

protected:

	void PollIncomingMessages() override;

public:

	bool isHostClient = false;
	bool isConnected = false;

private:

	HSteamNetConnection _connection;

};
