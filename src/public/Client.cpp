#include "Client.h"

#include <cassert>
#include <cstdio>
#include <string>
#include <steam/isteamnetworkingsockets.h>

#include "ByteStream.h"
#include "NetcodeUtilites.h"
#include "ObjectStateReader.h"

#include "windows.h"

HSteamNetConnection Client::_connection = k_HSteamNetConnection_Invalid;
bool Client::isHostClient = false;
uint8 Client::playerID = 255;

void Client::Start()
{
	NetCode::Start();

	if(isHostClient)
	{
		const std::string address = "127.0.0.1";
		SetIPv4Address(address);
	}

	char serverAddress[SteamNetworkingIPAddr::k_cchMaxString];
	addrServer.ToString(serverAddress, sizeof(serverAddress), true);
	printf("Connecting to server at %s \n", serverAddress);

	if (isHostClient)
		_connection = steamInterface->ConnectByIPAddress(addrServer, 0, nullptr);
	else
		_connection = steamInterface->ConnectByIPAddress(addrServer, 1, &configValue);

	if (_connection == k_HSteamNetConnection_Invalid)
		FatalError("Failed to Create Connection");
}

void Client::OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info)
{
	if(info->m_hConn == _connection || _connection == k_HSteamNetConnection_Invalid)
		assert(1);

	// What's the state of the connection?
	switch (info->m_info.m_eState)
	{
	case k_ESteamNetworkingConnectionState_None:
		// NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
		break;

	case k_ESteamNetworkingConnectionState_ClosedByPeer:
	case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
	{
		// Print an appropriate message
		if (info->m_eOldState == k_ESteamNetworkingConnectionState_Connecting)
		{
			// Note: we could distinguish between a timeout, a rejected connection,
			// or some other transport problem.
			printf("\n We sought the remote host, yet our efforts were met with defeat.  (%s)\n", info->m_info.m_szEndDebug);
		}
		else if (info->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
		{
			printf("\n Alas, troubles beset us; we have lost contact with the host.  (%s)\n", info->m_info.m_szEndDebug);
		}
		else
		{
			// NOTE: We could check the reason code for a normal disconnection
			printf("\n The host hath bidden us farewell.  (%s)\n", info->m_info.m_szEndDebug);
		}

		// Clean up the connection.  This is important!
		// The connection is "closed" in the network sense, but
		// it has not been destroyed.  We must close it on our end, too
		// to finish up.  The reason information do not matter in this case,
		// and we cannot linger because it's already closed on the other end,
		// so we just pass 0's.
		steamInterface->CloseConnection(info->m_hConn, 0, nullptr, false);
		_connection = k_HSteamNetConnection_Invalid;
		break;
	}

	case k_ESteamNetworkingConnectionState_Connecting:
		// We will get this callback when we start connecting.
		// We can ignore this.
		break;

	case k_ESteamNetworkingConnectionState_Connected:
		isConnected = true;
		ShowWindow(GetConsoleWindow(), SW_HIDE);
		break;

	default:
		break;
	}
}

/*
 *		Receive Message
 */

// Receive Message From Server
void Client::PollIncomingMessages()
{
	ISteamNetworkingMessage* messageList[1];
	int numMsgs = steamInterface->ReceiveMessagesOnConnection(_connection, messageList, 1);
	if (numMsgs == 0)
		return;
	if (numMsgs < 0)
		FatalError("Error checking for messages");

	for (int i = 0; i < numMsgs; i++)
	{
		const auto message = static_cast<char*>(messageList[i]->m_pData);
		if (message == nullptr) return;

		// Check if the message is a ByteStream
		if (message[0] == BYTE_STREAM_CODE)
		{
			if (IsHostClient()) return;
			ReadByteStream(messageList[i]->m_conn, message);
		}
		else
		{
			// Just echo anything we get from the server
			fwrite(messageList[i]->m_pData, 1, messageList[i]->m_cbSize, stdout);
			fputc('\n', stdout);
		}

		// We don't need this anymore.
		messageList[i]->Release();
	}
}

/*
 *		This function should never be called on the host client because the server manages the game state for them
 */
void Client::ReadByteStream(const HSteamNetConnection messageAuthor, const char* buffer)
{
	if (buffer[1] != SERVER_MESSAGE) { printf("Invalid Message Received!! \n"); return; }

	switch(static_cast<GSM_Server>(buffer[2]))
	{
	case GSM_Server::GSM_SpawnPlayer:
			ObjectStateReader::SpawnPlayer(buffer);
		break;

	case GSM_Server::GSM_WorldState:
			ObjectStateReader::WorldState(buffer);
		break;

	case GSM_Server::GSM_UpdateObject:
		ObjectStateReader::UpdateObject(buffer);
		break;
	case GSM_Server::GSM_UpdatePlayerList:
		ObjectStateReader::UpdatePlayerList(buffer);
		break;
	}
}

/*
 *		Send Message
 */

void Client::SendByteStreamToServer(const ByteStream& stream)
{
	if (steamInterface == nullptr) return;

	if (_connection == k_HSteamNetConnection_Invalid)
		assert(1);

	steamInterface->SendMessageToConnection(_connection, stream.buffer, stream.size,
		k_nSteamNetworkingSend_Reliable, nullptr);
}