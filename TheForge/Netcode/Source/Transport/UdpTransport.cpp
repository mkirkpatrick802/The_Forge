#include "UdpTransport.h"

#include <cstdlib>
#include <cstring>
#include <memory>
#include <ranges>
#include <vector>

#include "Engine/LaunchOptions.h"
#include "Engine/System.h"
#include "Engine/Time.h"

namespace
{
	// Number of small reliable messages the self-test pushes through the link.
	constexpr uint32_t SELF_TEST_MESSAGE_COUNT = 200;

	// Size of the oversized message that follows them, chosen to need dozens of
	// fragments so reassembly is genuinely exercised.
	constexpr uint32_t SELF_TEST_LARGE_BYTES = 50000;

	// Deterministic filler, so the receiver can verify the payload byte for byte
	// rather than merely checking its length.
	uint8_t SelfTestByte(const uint32_t index)
	{
		return static_cast<uint8_t>((index * 7 + 13) % 251);
	}
}

NetCode::UdpTransport& NetCode::UdpTransport::GetInstance()
{
	static auto instance = std::make_unique<UdpTransport>();
	return *instance;
}

bool NetCode::UdpTransport::StartServer(const uint16_t port)
{
	Shutdown();

	if (!_socket.Open(port))
	{
		DEBUG_LOG("Transport: could not start server on port %u.", port)
		return false;
	}

	_mode = ETM_Server;
	DEBUG_LOG("Transport: server listening on port %u.", _socket.GetBoundPort())

	if (const uint32_t loss = Engine::GetLaunchOptions().netLossPercent; loss > 0)
		DEBUG_LOG("Transport: simulating %u%% outgoing packet loss.", loss)

	return true;
}

bool NetCode::UdpTransport::StartClient(const std::string& host, const uint16_t port)
{
	Shutdown();

	if (!NetAddress::FromString(host, port, _serverAddress))
	{
		DEBUG_LOG("Transport: could not resolve server address '%s'.", host.c_str())
		return false;
	}

	// Port 0: the OS picks a free local port, which is what a client wants.
	if (!_socket.Open(0))
	{
		DEBUG_LOG("Transport: could not open a client socket.")
		return false;
	}

	const uint64_t now = Engine::Time::GetTicks();

	_mode = ETM_Client;
	_clientState = ECS_Connecting;
	_connectStartedMs = now;
	_lastRequestSentMs = 0; // forces a request on the first update
	_lastReceivedMs = now;

	DEBUG_LOG("Transport: connecting to %s...", _serverAddress.ToString().c_str())

	if (const uint32_t loss = Engine::GetLaunchOptions().netLossPercent; loss > 0)
		DEBUG_LOG("Transport: simulating %u%% outgoing packet loss.", loss)

	return true;
}

void NetCode::UdpTransport::Shutdown()
{
	// Losing the connection is a disconnect the game still has to react to, so the
	// event is recorded here and deliberately survives the reset below.
	const bool wasConnectedClient = _mode == ETM_Client && _clientState == ECS_Connected;

	if (_mode == ETM_Client && _clientState == ECS_Connected)
		SendHeaderOnly(_serverAddress, ETP_Disconnect, _connectionId, &_clientChannel);
	else if (_mode == ETM_Server)
		for (auto& connection : _connections | std::views::values)
			SendHeaderOnly(connection.address, ETP_Disconnect, connection.id, &connection.channel);

	_socket.Close();

	_mode = ETM_Inactive;
	_connections.clear();
	_nextConnectionId = 1;

	_clientState = ECS_Disconnected;
	_connectionId = 0;
	_serverAddress = NetAddress();
	_clientChannel = ReliableChannel();

	_received.clear();
	_connectedEvents.clear();
	_disconnectedEvents.clear();
	if (wasConnectedClient)
		_disconnectedEvents.push_back(0);

	_selfTestSent = 0;
	_selfTestReceived = 0;
	_selfTestComplete = false;
}

bool NetCode::UdpTransport::SendDatagram(const NetAddress& to, const void* data, const uint32_t size) const
{
	// Simulated loss applies to outgoing packets only. Loopback never drops
	// anything, so without this there is no way to exercise retransmission.
	if (const uint32_t loss = Engine::GetLaunchOptions().netLossPercent; loss > 0)
	{
		if (static_cast<uint32_t>(std::rand() % 100) < loss)
			return true; // pretend it went out
	}

	return _socket.SendTo(to, data, size);
}

bool NetCode::UdpTransport::SendHeaderOnly(const NetAddress& to, const ETransportPacketType type, const uint32_t connectionId, ReliableChannel* channel)
{
	TransportHeader header;
	header.type = type;
	header.connectionId = connectionId;

	// Even handshake and keep-alive packets carry acks, so acknowledgements keep
	// flowing during quiet periods.
	if (channel != nullptr)
		channel->StampOutgoing(header);

	return SendDatagram(to, &header, sizeof(header));
}

NetCode::RemoteConnection* NetCode::UdpTransport::FindConnection(const uint32_t connectionId)
{
	for (auto& connection : _connections | std::views::values)
		if (connection.id == connectionId)
			return &connection;

	return nullptr;
}

bool NetCode::UdpTransport::SendToServer(const ETransportChannel channel, const void* data, const uint32_t size)
{
	if (!IsConnected()) return false;

	std::vector<std::vector<uint8_t>> datagrams;
	if (!_clientChannel.QueuePayload(channel, data, size, _connectionId, datagrams))
		return false;

	const uint64_t now = Engine::Time::GetTicks();
	bool sent = true;

	for (const auto& datagram : datagrams)
	{
		TransportHeader header;
		std::memcpy(&header, datagram.data(), sizeof(header));

		sent &= SendDatagram(_serverAddress, datagram.data(), static_cast<uint32_t>(datagram.size()));
		_clientChannel.MarkSent(header.sequence, now);
	}

	_lastSentMs = now;
	return sent;
}

bool NetCode::UdpTransport::SendToConnection(const uint32_t connectionId, const ETransportChannel channel, const void* data, const uint32_t size)
{
	if (!IsServer()) return false;

	RemoteConnection* connection = FindConnection(connectionId);
	if (connection == nullptr) return false;

	std::vector<std::vector<uint8_t>> datagrams;
	if (!connection->channel.QueuePayload(channel, data, size, connection->id, datagrams))
		return false;

	const uint64_t now = Engine::Time::GetTicks();
	bool sent = true;

	for (const auto& datagram : datagrams)
	{
		TransportHeader header;
		std::memcpy(&header, datagram.data(), sizeof(header));

		sent &= SendDatagram(connection->address, datagram.data(), static_cast<uint32_t>(datagram.size()));
		connection->channel.MarkSent(header.sequence, now);
	}

	connection->lastSentMs = now;
	return sent;
}

void NetCode::UdpTransport::SendToAll(const ETransportChannel channel, const void* data, const uint32_t size)
{
	if (!IsServer()) return;

	for (auto& connection : _connections | std::views::values)
		SendToConnection(connection.id, channel, data, size);
}

std::vector<uint32_t> NetCode::UdpTransport::GetConnectionIds() const
{
	std::vector<uint32_t> ids;
	ids.reserve(_connections.size());
	for (const auto& connection : _connections | std::views::values)
		ids.push_back(connection.id);

	return ids;
}

std::vector<NetCode::ReceivedMessage> NetCode::UdpTransport::TakeReceivedMessages()
{
	std::vector<ReceivedMessage> out;
	out.swap(_received);
	return out;
}

void NetCode::UdpTransport::KickConnection(const uint32_t connectionId)
{
	if (_mode != ETM_Server) return;

	for (auto it = _connections.begin(); it != _connections.end(); ++it)
	{
		if (it->second.id != connectionId) continue;

		// Best effort: the goodbye turns a kick into an immediate, explained
		// disconnect on the client rather than a five second silence.
		SendHeaderOnly(it->second.address, ETP_Disconnect, connectionId, &it->second.channel);

		_connections.erase(it);
		_disconnectedEvents.push_back(connectionId);
		DEBUG_LOG("Transport: kicked client %u (%u remaining).", connectionId, GetConnectionCount())
		return;
	}
}

void NetCode::UdpTransport::TakeConnectionEvents(std::vector<uint32_t>& outConnected, std::vector<uint32_t>& outDisconnected)
{
	outConnected = std::move(_connectedEvents);
	outDisconnected = std::move(_disconnectedEvents);
	_connectedEvents.clear();
	_disconnectedEvents.clear();
}

void NetCode::UdpTransport::Update()
{
	if (!IsActive()) return;

	ReceivePackets();

	const uint64_t now = Engine::Time::GetTicks();
	if (_mode == ETM_Server)
		UpdateServer(now);
	else
		UpdateClient(now);

	if (Engine::GetLaunchOptions().netSelfTest)
		UpdateSelfTest(now);
}

void NetCode::UdpTransport::ReceivePackets()
{
	uint8_t buffer[MAX_TRANSPORT_PACKET_BYTES];
	NetAddress from;

	// Bounded so a flood cannot stall the frame.
	for (uint32_t i = 0; i < MAX_RECEIVE_PER_TICK; ++i)
	{
		uint32_t received = 0;
		const UdpSocket::EReceiveResult result = _socket.ReceiveFrom(from, buffer, sizeof(buffer), received);

		// Only an empty queue ends the drain. A datagram that failed says nothing about
		// the ones behind it, and stopping on one used to cost every other peer their
		// packets for the frame.
		if (result == UdpSocket::EReceiveResult::Empty) break;
		if (result == UdpSocket::EReceiveResult::Skipped) continue;

		if (received < sizeof(TransportHeader))
			continue; // too small to be ours

		TransportHeader header;
		std::memcpy(&header, buffer, sizeof(header));

		// Drops stray traffic and mismatched builds before anything is interpreted.
		if (header.protocolId != TRANSPORT_PROTOCOL_ID)
			continue;

		if (header.type >= ETP_Max)
			continue;

		HandlePacket(from, header, buffer + sizeof(header), received - static_cast<uint32_t>(sizeof(header)));
	}
}

void NetCode::UdpTransport::HandlePacket(const NetAddress& from, const TransportHeader& header, const uint8_t* payload, const uint32_t payloadSize)
{
	if (_mode == ETM_Server)
		HandleServerPacket(from, header, payload, payloadSize);
	else
		HandleClientPacket(from, header, payload, payloadSize);
}

void NetCode::UdpTransport::HandleServerPacket(const NetAddress& from, const TransportHeader& header, const uint8_t* payload, const uint32_t payloadSize)
{
	const uint64_t now = Engine::Time::GetTicks();
	const auto existing = _connections.find(from);

	if (header.type == ETP_ConnectRequest)
	{
		if (existing != _connections.end())
		{
			// A repeated request means the accept was lost; answer it again.
			existing->second.lastReceivedMs = now;
			existing->second.lastSentMs = now;
			SendHeaderOnly(from, ETP_ConnectAccept, existing->second.id, &existing->second.channel);
			return;
		}

		if (_connections.size() >= MAX_CONNECTIONS)
		{
			DEBUG_LOG("Transport: refused %s, server full.", from.ToString().c_str())
			SendHeaderOnly(from, ETP_ConnectDeny, 0, nullptr);
			return;
		}

		RemoteConnection connection;
		connection.id = _nextConnectionId++;
		connection.address = from;
		connection.lastReceivedMs = now;
		connection.lastSentMs = now;

		const auto inserted = _connections.emplace(from, std::move(connection)).first;
		SendHeaderOnly(from, ETP_ConnectAccept, inserted->second.id, &inserted->second.channel);

		_connectedEvents.push_back(inserted->second.id);
		DEBUG_LOG("Transport: client %u connected from %s (%u total).", inserted->second.id, from.ToString().c_str(), GetConnectionCount())
		return;
	}

	// Everything else must come from a peer we already know.
	if (existing == _connections.end())
		return;

	RemoteConnection& connection = existing->second;
	connection.lastReceivedMs = now;
	connection.channel.ProcessIncomingHeader(header);

	if (header.type == ETP_Disconnect)
	{
		const uint32_t id = connection.id;
		_connections.erase(existing);
		_disconnectedEvents.push_back(id);
		DEBUG_LOG("Transport: client %u disconnected (%u remaining).", id, GetConnectionCount())
		return;
	}

	if (header.type == ETP_Payload)
	{
		std::vector<TransportMessage> delivered;
		connection.channel.ProcessIncomingPayload(header, payload, payloadSize, delivered);

		for (auto& message : delivered)
			_received.push_back({ connection.id, std::move(message) });
	}

	// Sent from inside the receive loop on purpose: waiting for the end of the
	// frame is exactly what puts the front of a large burst out of ack range.
	if (connection.channel.NeedsImmediateAck())
	{
		SendHeaderOnly(from, ETP_KeepAlive, connection.id, &connection.channel);
		connection.lastSentMs = now;
	}
}

void NetCode::UdpTransport::HandleClientPacket(const NetAddress& from, const TransportHeader& header, const uint8_t* payload, const uint32_t payloadSize)
{
	// Only the server we asked for is allowed to talk to us.
	if (from != _serverAddress) return;

	_lastReceivedMs = Engine::Time::GetTicks();

	switch (header.type)
	{
	case ETP_ConnectAccept:
		if (_clientState != ECS_Connected)
		{
			_clientState = ECS_Connected;
			_connectionId = header.connectionId;
			_connectedEvents.push_back(0);
			DEBUG_LOG("Transport: connected to %s as client %u.", _serverAddress.ToString().c_str(), _connectionId)
		}
		_clientChannel.ProcessIncomingHeader(header);
		break;

	case ETP_ConnectDeny:
		DEBUG_LOG("Transport: connection refused by %s.", _serverAddress.ToString().c_str())
		Shutdown();
		break;

	case ETP_Disconnect:
		DEBUG_LOG("Transport: server closed the connection.")
		Shutdown();
		break;

	case ETP_KeepAlive:
		_clientChannel.ProcessIncomingHeader(header);
		break;

	case ETP_Payload:
	{
		_clientChannel.ProcessIncomingHeader(header);

		std::vector<TransportMessage> delivered;
		_clientChannel.ProcessIncomingPayload(header, payload, payloadSize, delivered);

		for (auto& message : delivered)
			_received.push_back({ 0, std::move(message) });

		// See the matching call on the server side: a burst larger than the ack
		// window has to be acknowledged as it arrives, not once it has all landed.
		if (_clientChannel.NeedsImmediateAck())
		{
			SendHeaderOnly(_serverAddress, ETP_KeepAlive, _connectionId, &_clientChannel);
			_lastSentMs = Engine::Time::GetTicks();
		}
		break;
	}

	default:
		break;
	}
}

void NetCode::UdpTransport::UpdateServer(const uint64_t nowMs)
{
	for (auto it = _connections.begin(); it != _connections.end();)
	{
		RemoteConnection& connection = it->second;

		if (nowMs - connection.lastReceivedMs > CONNECTION_TIMEOUT_MS)
		{
			DEBUG_LOG("Transport: client %u timed out.", connection.id)
			_disconnectedEvents.push_back(connection.id);
			it = _connections.erase(it);
			continue;
		}

		std::vector<const std::vector<uint8_t>*> resends;
		connection.channel.CollectResends(nowMs, resends);
		for (const auto* datagram : resends)
		{
			SendDatagram(connection.address, datagram->data(), static_cast<uint32_t>(datagram->size()));
			connection.lastSentMs = nowMs;
		}

		// Either we owe the peer an ack, or the connection has gone quiet long
		// enough to need a heartbeat. Both ride the same packet.
		if (connection.channel.HasUnsentAcks() || nowMs - connection.lastSentMs >= KEEP_ALIVE_INTERVAL_MS)
		{
			SendHeaderOnly(connection.address, ETP_KeepAlive, connection.id, &connection.channel);
			connection.lastSentMs = nowMs;
		}

		++it;
	}
}

void NetCode::UdpTransport::UpdateClient(const uint64_t nowMs)
{
	if (_clientState == ECS_Connecting)
	{
		if (nowMs - _connectStartedMs > CONNECT_TIMEOUT_MS)
		{
			DEBUG_LOG("Transport: timed out connecting to %s.", _serverAddress.ToString().c_str())
			Shutdown();
			return;
		}

		if (nowMs - _lastRequestSentMs >= CONNECT_REQUEST_INTERVAL_MS)
		{
			SendHeaderOnly(_serverAddress, ETP_ConnectRequest, 0, &_clientChannel);
			_lastRequestSentMs = nowMs;
			_lastSentMs = nowMs;
		}

		return;
	}

	if (_clientState != ECS_Connected) return;

	if (nowMs - _lastReceivedMs > CONNECTION_TIMEOUT_MS)
	{
		DEBUG_LOG("Transport: lost connection to the server.")
		Shutdown();
		return;
	}

	std::vector<const std::vector<uint8_t>*> resends;
	_clientChannel.CollectResends(nowMs, resends);
	for (const auto* datagram : resends)
	{
		SendDatagram(_serverAddress, datagram->data(), static_cast<uint32_t>(datagram->size()));
		_lastSentMs = nowMs;
	}

	// Same as the server side: ack promptly, or the server's send window fills
	// while this client sits on a second's worth of unacknowledged world state.
	if (_clientChannel.HasUnsentAcks() || nowMs - _lastSentMs >= KEEP_ALIVE_INTERVAL_MS)
	{
		SendHeaderOnly(_serverAddress, ETP_KeepAlive, _connectionId, &_clientChannel);
		_lastSentMs = nowMs;
	}
}

// --- self-test -------------------------------------------------------------
// The client pushes a fixed run of numbered reliable messages at the server,
// which checks they arrive exactly once each and in order. Combined with
// --net-loss this is what actually demonstrates retransmission and ordering;
// loopback alone never drops a packet.

void NetCode::UdpTransport::UpdateSelfTest(uint64_t)
{
	if (IsClient() && IsConnected() && !_selfTestComplete)
	{
		// A few per tick, so the send window is exercised rather than overrun.
		for (int i = 0; i < 4 && _selfTestSent < SELF_TEST_MESSAGE_COUNT; ++i)
		{
			const uint32_t index = _selfTestSent;
			if (!SendToServer(ETC_Reliable, &index, sizeof(index)))
				break;

			++_selfTestSent;
		}

		if (_selfTestSent == SELF_TEST_MESSAGE_COUNT)
		{
			DEBUG_LOG("Self-test: sent all %u reliable messages.", SELF_TEST_MESSAGE_COUNT)

			// Now one message far larger than a datagram, to exercise fragmentation.
			std::vector<uint8_t> large(SELF_TEST_LARGE_BYTES);
			for (uint32_t i = 0; i < SELF_TEST_LARGE_BYTES; ++i)
				large[i] = SelfTestByte(i);

			if (SendToServer(ETC_Reliable, large.data(), SELF_TEST_LARGE_BYTES))
				DEBUG_LOG("Self-test: sent a %u byte message (%u fragments).", SELF_TEST_LARGE_BYTES, (SELF_TEST_LARGE_BYTES + MAX_PAYLOAD_BYTES - 1) / MAX_PAYLOAD_BYTES)
			else
				DEBUG_LOG("Self-test FAILED: could not queue the large message.")

			_selfTestComplete = true;
		}
	}

	for (const auto& message : TakeReceivedMessages())
		HandleSelfTestMessage(message);
}

void NetCode::UdpTransport::HandleSelfTestMessage(const ReceivedMessage& message)
{
	// The large fragmented message, which arrives after the small ones.
	if (message.data.size() == SELF_TEST_LARGE_BYTES)
	{
		for (uint32_t i = 0; i < SELF_TEST_LARGE_BYTES; ++i)
		{
			if (message.data[i] == SelfTestByte(i)) continue;

			DEBUG_LOG("Self-test FAILED: reassembled message differs at byte %u.", i)
			return;
		}

		DEBUG_LOG("Self-test PASSED: %u byte message reassembled intact from %u fragments.", SELF_TEST_LARGE_BYTES, (SELF_TEST_LARGE_BYTES + MAX_PAYLOAD_BYTES - 1) / MAX_PAYLOAD_BYTES)
		return;
	}

	if (message.data.size() != sizeof(uint32_t)) return;
	if (_selfTestReceived >= SELF_TEST_MESSAGE_COUNT) return;

	uint32_t index = 0;
	std::memcpy(&index, message.data.data(), sizeof(index));

	if (index != _selfTestReceived)
	{
		DEBUG_LOG("Self-test FAILED: expected message %u but got %u.", _selfTestReceived, index)
		_selfTestReceived = SELF_TEST_MESSAGE_COUNT; // stop reporting
		return;
	}

	++_selfTestReceived;

	if (_selfTestReceived == SELF_TEST_MESSAGE_COUNT)
		DEBUG_LOG("Self-test PASSED: %u reliable messages, all in order, no duplicates.", SELF_TEST_MESSAGE_COUNT)
	else if (_selfTestReceived % 50 == 0)
		DEBUG_LOG("Self-test: %u/%u received in order.", _selfTestReceived, SELF_TEST_MESSAGE_COUNT)
}
