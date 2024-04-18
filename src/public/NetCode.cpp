//
// Created by mKirkpatrick on 2/24/2024.
//

#include "NetCode.h"

#include <cstdarg>
#include <cstdio>
#include <SDL_syswm.h>
#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

SteamNetworkingMicroseconds g_logTimeZero;

NetCode* NetCode::_callbackInstance = nullptr;
ISteamNetworkingSockets* NetCode::steamInterface = nullptr;

NetCode::NetCode()
{

}

void NetCode::Init()
{
	SteamDatagramErrMsg errMsg;
	if (!GameNetworkingSockets_Init(nullptr, errMsg))
		FatalError("GameNetworkingSockets_Init failed.  %s", errMsg);

	g_logTimeZero = SteamNetworkingUtils()->GetLocalTimestamp();
	SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugOutput);
}

void NetCode::Start()
{
	steamInterface = SteamNetworkingSockets();
	configValue.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetConnectionStatusChangedCallback);
}

void NetCode::Update()
{
	PollIncomingMessages();
	PollConnectionStateChange();
}

void NetCode::PollConnectionStateChange()
{
	_callbackInstance = this;
	steamInterface->RunCallbacks();
}

void NetCode::SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* info)
{
	_callbackInstance->OnSteamNetConnectionStatusChanged(info);
}

void NetCode::FatalError(const char* fmt, ...)
{
	char text[2048];
	va_list ap;
	va_start(ap, fmt);
	vsprintf_s(text, fmt, ap);
	va_end(ap);
	char* nl = strchr(text, '\0') - 1;
	if (nl >= text && *nl == '\n')
		*nl = '\0';
	DebugOutput(k_ESteamNetworkingSocketsDebugOutputType_Bug, text);
}

void NetCode::DebugOutput(ESteamNetworkingSocketsDebugOutputType eType, const char* pszMsg)
{
	SteamNetworkingMicroseconds time = SteamNetworkingUtils()->GetLocalTimestamp() - g_logTimeZero;
	printf("%10.6f %s\n", time * 1e-6, pszMsg);
	fflush(stdout);
	if (eType == k_ESteamNetworkingSocketsDebugOutputType_Bug)
	{
		fflush(stdout);
		fflush(stderr);
		ExitProcess(1);
	}
}

void NetCode::SetIPv4Address(std::string address)
{
	if (!addrServer.ParseString(address.c_str()))
		NetCode::FatalError("Invalid server address %s", address.c_str());

	if (addrServer.m_port == 0)
		addrServer.m_port = DEFAULT_SERVER_PORT;
}