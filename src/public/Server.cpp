  #include "Server.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <steam/isteamnetworkingsockets.h>

#include "ByteStream.h"
#include "Client.h"

Server::Server()
{

}

void Server::Start()
{
	NetCode::Start();

	addrServer.Clear();
	addrServer.m_port = DEFAULT_SERVER_PORT;
	_listenSocket = steamInterface->CreateListenSocketIP(addrServer, 1, &configValue);

	char serverAddress[SteamNetworkingIPAddr::k_cchMaxString];
	addrServer.ToString(serverAddress, sizeof(serverAddress), true);

	if(_listenSocket == k_HSteamListenSocket_Invalid)
		FatalError("Failed to listen on port %d", addrServer.m_port);

	_pollGroup = steamInterface->CreatePollGroup();
	if(_pollGroup == k_HSteamNetPollGroup_Invalid)
		FatalError("Failed to listen on port %d", addrServer.m_port);

	printf(" \nServer listening on port %d \n", addrServer.m_port);

	_client = new Client();
	_client->isHostClient = true;
	_client->Start();
}

void Server::Update()
{
	NetCode::Update();

	_client->Update();
}

// Read Message From Clients
void Server::PollIncomingMessages()
{
	for (const auto& client : _mapClients)
	{
		ISteamNetworkingMessage* pIncomingMsg = nullptr;
		int numMsgs = steamInterface->ReceiveMessagesOnConnection(client.first, &pIncomingMsg, 1);
		if (numMsgs == 0)
			return;
		if (numMsgs < 0)
			FatalError("Error checking for messages");

		if (auto message = static_cast<char*>(pIncomingMsg->m_pData); message[0] == BYTE_STREAM_CODE)
			ReadByteStream(message);
	}
}

void Server::ReadByteStream(const char* buffer)
{
	if (buffer[1] != CLIENT_MESSAGE) { printf("Invalid Message Received!! \n"); return; }

	switch((GSM_Client)buffer[2])
	{
	case GSM_Client::GSM_SyncWorld:

		printf("Sync World \n");

		break;
	case GSM_Client::GSM_MovementInput:
		break;
	}
}

void Server::OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info)
{
	char temp[1024];

	// What's the state of the connection?
	switch (info->m_info.m_eState)
	{
	case k_ESteamNetworkingConnectionState_None:
		// NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
		break;

	case k_ESteamNetworkingConnectionState_ClosedByPeer:
	case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
	{
		// Ignore if they were not previously connected.  (If they disconnected
		// before we accepted the connection.)
		if (info->m_eOldState == k_ESteamNetworkingConnectionState_Connected)
		{

			// Locate the client.  Note that it should have been found, because this
			// is the only codepath where we remove clients (except on shutdown),
			// and connection change callbacks are dispatched in queue order.
			auto itClient = _mapClients.find(info->m_hConn);
			assert(itClient != _mapClients.end());

			// Select appropriate log messages
			const char* pszDebugLogAction;
			if (info->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
			{
				pszDebugLogAction = "problem detected locally";
				sprintf_s(temp, "Alas, %s hath fallen into shadow.  (%s)", itClient->second.nickname.c_str(), info->m_info.m_szEndDebug);
			}
			else
			{
				// Note that here we could check the reason code to see if
				// it was a "usual" connection or an "unusual" one.
				pszDebugLogAction = "closed by peer";
				sprintf_s(temp, "%s hath departed", itClient->second.nickname.c_str());
			}

			// Spew something to our own log.  Note that because we put their nick
			// as the connection description, it will show up, along with their
			// transport-specific data (e.g. their IP address)
			printf("Connection %s %s, reason %d: %s\n",
				info->m_info.m_szConnectionDescription,
				pszDebugLogAction,
				info->m_info.m_eEndReason,
				info->m_info.m_szEndDebug
			);

			_mapClients.erase(itClient);

			// Send a message so everybody else knows what happened
			SendStringToAllClients(temp);

			//TODO: Remove player game object from clients world
		}
		else
		{
			assert(info->m_eOldState == k_ESteamNetworkingConnectionState_Connecting);
		}

		// Clean up the connection.  This is important!
		// The connection is "closed" in the network sense, but
		// it has not been destroyed.  We must close it on our end, too
		// to finish up.  The reason information do not matter in this case,
		// and we cannot linger because it's already closed on the other end,
		// so we just pass 0's.
		steamInterface->CloseConnection(info->m_hConn, 0, nullptr, false);
		break;
	}

	case k_ESteamNetworkingConnectionState_Connecting:
	{
		// This must be a new connection
		assert(_mapClients.find(info->m_hConn) == _mapClients.end());

		printf("Connection request from %s \n", info->m_info.m_szConnectionDescription);

		// A client is attempting to connect
		// Try to accept the connection.
		if (steamInterface->AcceptConnection(info->m_hConn) != k_EResultOK)
		{
			// This could fail.  If the remote host tried to connect, but then
			// disconnected, the connection may already be half closed.  Just
			// destroy whatever we have on our side.
			steamInterface->CloseConnection(info->m_hConn, 0, nullptr, false);
			printf("Can't accept connection.  (It was already closed?)");
			break;
		}

		// Assign the poll group
		if (!steamInterface->SetConnectionPollGroup(info->m_hConn, _pollGroup))
		{
			steamInterface->CloseConnection(info->m_hConn, 0, nullptr, false);
			printf("Failed to set poll group?");
			break;
		}

		// Generate a random nick.  A random temporary nick
		// is really dumb and not how you would write a real chat server.
		// You would want them to have some sort of signon message,
		// and you would keep their client in a state of limbo (connected,
		// but not logged on) until them.  I'm trying to keep this example
		// code really simple.
		char nick[64];
		sprintf_s(nick, "BraveWarrior%d", 10000 + (rand() % 100000));

		// Send them a welcome message
		sprintf_s(temp, "Welcome, stranger.  Thou art known to us for now as '%s'", nick);
		SendStringToClient(info->m_hConn, temp);

		// Also send them a list of everybody who is already connected
		if (_mapClients.empty())
		{
			SendStringToClient(info->m_hConn, "Thou art utterly alone.");
		}
		else
		{
			sprintf_s(temp, "%d companions greet you:", (int)_mapClients.size());
			SendStringToClient(info->m_hConn, temp);
			for (auto& c : _mapClients)
				SendStringToClient(info->m_hConn, c.second.nickname.c_str());
		}

		// Let everybody else know who they are for now
		sprintf_s(temp, "Hark!  A stranger hath joined this merry host.  For now we shall call them '%s'", nick);
		SendStringToAllClients(temp, info->m_hConn);

		// Add them to the client list, using std::map wacky syntax
		_mapClients[info->m_hConn];
		SetClientNickname(info->m_hConn, nick);

		// Create a Bytestream to tell clients to spawn a player
		ByteStream stream;
		stream.WriteGSM(GSM_Server::GSM_SpawnPlayer);
		SendByteSteamToAllClients(stream);
		break;
	}

	case k_ESteamNetworkingConnectionState_Connected:
		// We will get a callback immediately after accepting the connection.
		// Since we are the server, we can ignore this, it's not news to us.
		break;

	default:
		break;
	}
}

bool Server::IsServerClientConnected() const
{
	return _client->isConnected;
}

void Server::SetClientNickname(HSteamNetConnection connection, const char* nickname)
{
	_mapClients[connection].nickname = nickname;
	steamInterface->SetConnectionName(connection, nickname);
}

void Server::SendStringToClient(HSteamNetConnection connection, const char* str) const
{
	steamInterface->SendMessageToConnection(connection, str, strlen(str), k_nSteamNetworkingSend_Reliable, nullptr);
}

void Server::SendStringToClient(HSteamNetConnection connection, const char* str, const int size) const
{
	steamInterface->SendMessageToConnection(connection, str, size, k_nSteamNetworkingSend_Reliable, nullptr);
}

void Server::SendStringToAllClients(const char* str, HSteamNetConnection except)
{
	for (const auto& clients : _mapClients)
	{
		if (clients.first != except)
			SendStringToClient(clients.first, str);
	}
}

void Server::SendByteSteamToAllClients(const ByteStream& byteStream)
{
	for (const auto& clients : _mapClients)
	{
		SendStringToClient(clients.first, byteStream.buffer, byteStream.size);
	}
}