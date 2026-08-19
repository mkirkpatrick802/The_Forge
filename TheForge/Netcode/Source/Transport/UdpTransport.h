#pragma once
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "NetAddress.h"
#include "ReliableChannel.h"
#include "TransportProtocol.h"
#include "UdpSocket.h"

namespace NetCode
{
	enum ETransportMode : uint8_t
	{
		ETM_Inactive = 0,
		ETM_Server,
		ETM_Client,
	};

	enum EClientState : uint8_t
	{
		ECS_Disconnected = 0,
		ECS_Connecting,
		ECS_Connected,
	};

	// A payload handed up to the game. connectionId identifies the sender on a
	// server, and is 0 on a client (everything comes from the server).
	struct ReceivedMessage
	{
		uint32_t connectionId = 0;
		TransportMessage data;
	};

	// One connected peer, as seen by the server.
	struct RemoteConnection
	{
		uint32_t id = 0;
		NetAddress address;
		uint64_t lastReceivedMs = 0;
		uint64_t lastSentMs = 0;
		ReliableChannel channel;
	};

	// UDP transport for the dedicated server model: connection setup, keep-alives,
	// timeouts, reliability and fragmentation.
	//
	// Deliberately independent of Steam, and deliberately ignorant of the game: it
	// moves opaque payloads and reports who came and went. UdpNetTransport is the
	// INetTransport wrapper that hands those to NetworkManager.
	class UdpTransport
	{
	public:
		static UdpTransport& GetInstance();

		bool StartServer(uint16_t port);
		bool StartClient(const std::string& host, uint16_t port);
		void Shutdown();

		// Pumps the socket: reads packets, answers handshakes, retransmits unacked
		// reliable packets, sends keep-alives and drops silent peers.
		void Update();

		ETransportMode GetMode() const { return _mode; }
		bool IsServer() const { return _mode == ETM_Server; }
		bool IsClient() const { return _mode == ETM_Client; }
		bool IsActive() const { return _mode != ETM_Inactive; }

		EClientState GetClientState() const { return _clientState; }
		bool IsConnected() const { return _mode == ETM_Client && _clientState == ECS_Connected; }
		uint32_t GetConnectionId() const { return _connectionId; }
		uint32_t GetConnectionCount() const { return static_cast<uint32_t>(_connections.size()); }
		// Every currently connected client, by id. Server-side only.
		std::vector<uint32_t> GetConnectionIds() const;

		// --- payloads ---
		// Client -> server.
		bool SendToServer(ETransportChannel channel, const void* data, uint32_t size);
		// Server -> one client, or all of them.
		bool SendToConnection(uint32_t connectionId, ETransportChannel channel, const void* data, uint32_t size);
		void SendToAll(ETransportChannel channel, const void* data, uint32_t size);

		// Messages delivered since the last call. Reliable ones arrive in order and
		// without duplicates; unreliable ones arrive however they turn up.
		std::vector<ReceivedMessage> TakeReceivedMessages();

		// Drops one client, telling it so rather than leaving it to time out. Reported
		// through TakeConnectionEvents like any other disconnect. Server-side only.
		void KickConnection(uint32_t connectionId);

		// Connections opened and closed since the last call, as raw connection ids.
		// A client reports the server as id 0, matching how ReceivedMessage tags it.
		// Drained rather than queried so no event can be missed between frames.
		void TakeConnectionEvents(std::vector<uint32_t>& outConnected, std::vector<uint32_t>& outDisconnected);

	private:
		void ReceivePackets();
		void HandlePacket(const NetAddress& from, const TransportHeader& header, const uint8_t* payload, uint32_t payloadSize);
		void HandleServerPacket(const NetAddress& from, const TransportHeader& header, const uint8_t* payload, uint32_t payloadSize);
		void HandleClientPacket(const NetAddress& from, const TransportHeader& header, const uint8_t* payload, uint32_t payloadSize);

		void UpdateServer(uint64_t nowMs);
		void UpdateClient(uint64_t nowMs);

		bool SendHeaderOnly(const NetAddress& to, ETransportPacketType type, uint32_t connectionId, ReliableChannel* channel);

		// Single exit point for every datagram, so the loss simulator only has to
		// exist in one place.
		bool SendDatagram(const NetAddress& to, const void* data, uint32_t size) const;

		RemoteConnection* FindConnection(uint32_t connectionId);

		// Exercises reliable delivery under simulated loss. See --net-selftest.
		void UpdateSelfTest(uint64_t nowMs);
		void HandleSelfTestMessage(const ReceivedMessage& message);

	private:
		UdpSocket _socket;
		ETransportMode _mode = ETM_Inactive;

		// Server
		std::map<NetAddress, RemoteConnection> _connections;
		uint32_t _nextConnectionId = 1;

		// Client
		NetAddress _serverAddress;
		EClientState _clientState = ECS_Disconnected;
		uint32_t _connectionId = 0;
		uint64_t _connectStartedMs = 0;
		uint64_t _lastRequestSentMs = 0;
		uint64_t _lastReceivedMs = 0;
		uint64_t _lastSentMs = 0;
		ReliableChannel _clientChannel;

		std::vector<ReceivedMessage> _received;
		std::vector<uint32_t> _connectedEvents;
		std::vector<uint32_t> _disconnectedEvents;

		// Self-test
		uint32_t _selfTestSent = 0;
		uint32_t _selfTestReceived = 0;
		bool _selfTestComplete = false;
	};

	inline UdpTransport& GetUdpTransport()
	{
		return UdpTransport::GetInstance();
	}
}
