﻿#pragma once

#include "NetCode.h"

class ByteStream;

const int MAX_CLIENT_MESSAGES = 3;

class Client : public NetCode
{
public:

	virtual void Start() override;
	virtual void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info) override;

	static void SendByteStreamToServer(const ByteStream& stream);
	static bool IsHostClient() { return isHostClient; }

	~Client() override;

protected:

	virtual void PollIncomingMessages() override;
	virtual void ReadByteStream(HSteamNetConnection messageAuthor, const char* buffer) override;

public:

	static uint8 playerID;
	static bool isHostClient;
	bool isConnected = false;

private:

	static HSteamNetConnection _connection;
};
