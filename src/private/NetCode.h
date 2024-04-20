//
// Created by mKirkpatrick on 2/24/2024.
//

#ifndef THE_FORGE_NETCODE_H
#define THE_FORGE_NETCODE_H

#include <string>
#include <steam/steamnetworkingtypes.h>

class ISteamNetworkingSockets;

constexpr uint16 DEFAULT_SERVER_PORT = 11111;

class NetCode
{
public:

	NetCode();
	static void Init();
	virtual void Start();

	virtual void Update();

	static void FatalError(const char* fmt, ...);
	static void DebugOutput(ESteamNetworkingSocketsDebugOutputType eType, const char* pszMsg);

	void SetIPv4Address(std::string address);

protected:

	// Callback
	static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* info);

	// Bind Function
	virtual void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info) = 0;

	virtual void PollIncomingMessages() = 0;

	virtual void ReadByteStream(const char* buffer, HSteamNetConnection messageAuthor) = 0;

private:

	void PollConnectionStateChange();

public:

	SteamNetworkingIPAddr addrServer;

protected:

	SteamNetworkingConfigValue_t configValue;
	static ISteamNetworkingSockets* steamInterface;

private:

	static NetCode* _callbackInstance;

};


#endif THE_FORGE_NETCODE_H
