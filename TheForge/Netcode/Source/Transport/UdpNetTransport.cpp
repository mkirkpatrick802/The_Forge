#include "UdpNetTransport.h"

#include <algorithm>

#include "Engine/LaunchOptions.h"

// The authority holds one replication slot per connected client, so the transport
// must never accept more clients than there are slots to track them with.
static_assert(NetCode::MAX_CONNECTIONS <= NetCode::MAX_REPLICATION_PEERS,
              "MAX_CONNECTIONS exceeds the per-peer replication mask width");

bool NetCode::UdpNetTransport::Start()
{
	const Engine::LaunchOptions& options = Engine::GetLaunchOptions();

	if (options.IsDedicatedServer())
		return GetUdpTransport().StartServer(options.port);

	if (options.IsClient())
		return GetUdpTransport().StartClient(options.serverAddress, options.port);

	return false;
}

void NetCode::UdpNetTransport::Shutdown()
{
	GetUdpTransport().Shutdown();
}

void NetCode::UdpNetTransport::Update()
{
	GetUdpTransport().Update();
}

bool NetCode::UdpNetTransport::IsClientRole()
{
	return Engine::GetLaunchOptions().IsClient();
}

NetCode::PeerID NetCode::UdpNetTransport::GetLocalPeerID() const
{
	// A client's identity is the id the server assigned it, which is also what the
	// server stamped on its pawn -- that is what makes IsLocalPlayer() agree with
	// the replicated world state. It stays INVALID_PEER until the accept arrives.
	if (IsClientRole())
		return GetUdpTransport().GetConnectionId();

	return SERVER_PEER_ID;
}

bool NetCode::UdpNetTransport::IsSessionActive() const
{
	const UdpTransport& transport = GetUdpTransport();
	return IsClientRole() ? transport.IsConnected() : transport.IsActive();
}

NetCode::PeerID NetCode::UdpNetTransport::ToPeerID(const uint32_t connectionId)
{
	return IsClientRole() ? SERVER_PEER_ID : static_cast<PeerID>(connectionId);
}

std::vector<NetCode::PeerID> NetCode::UdpNetTransport::GetRemotePeers() const
{
	const UdpTransport& transport = GetUdpTransport();

	if (IsClientRole())
		return transport.IsConnected() ? std::vector<PeerID>{ SERVER_PEER_ID } : std::vector<PeerID>{};

	std::vector<PeerID> peers;
	for (const uint32_t id : transport.GetConnectionIds())
		peers.push_back(static_cast<PeerID>(id));

	return peers;
}

bool NetCode::UdpNetTransport::HasPeer(const PeerID peer) const
{
	const std::vector<PeerID> peers = GetRemotePeers();
	return std::ranges::find(peers, peer) != peers.end();
}

void NetCode::UdpNetTransport::Disconnect(const PeerID peer)
{
	// Only a server kicks, and only a real client id. SERVER_PEER_ID here would mean a
	// client trying to disconnect the server, which is not a thing this direction does.
	if (!GetUdpTransport().IsServer()) return;
	if (peer == SERVER_PEER_ID || peer == INVALID_PEER) return;

	GetUdpTransport().KickConnection(static_cast<uint32_t>(peer));
}

bool NetCode::UdpNetTransport::SendTo(const PeerID peer, const OutputByteStream& stream, const bool reliable)
{
	UdpTransport& transport = GetUdpTransport();
	const ETransportChannel channel = reliable ? ETC_Reliable : ETC_Unreliable;
	const uint32_t size = stream.GetByteLength();

	if (IsClientRole())
	{
		// A client has exactly one peer, so anything addressed elsewhere is a bug
		// upstream rather than something to quietly route to the server anyway.
		if (peer != SERVER_PEER_ID) return false;
		return transport.SendToServer(channel, stream.GetBuffer(), size);
	}

	return transport.SendToConnection(static_cast<uint32_t>(peer), channel, stream.GetBuffer(), size);
}

std::vector<NetCode::NetMessage> NetCode::UdpNetTransport::Receive()
{
	std::vector<NetMessage> messages;

	for (const auto& received : GetUdpTransport().TakeReceivedMessages())
	{
		// Already whole: the transport reassembled any fragments before handing it up.
		messages.push_back({ ToPeerID(received.connectionId),
			InputByteStream(received.data.data(), static_cast<uint32_t>(received.data.size())) });
	}

	return messages;
}

std::vector<NetCode::PeerID> NetCode::UdpNetTransport::TakeConnectedPeers()
{
	std::vector<uint32_t> connected, disconnected;
	GetUdpTransport().TakeConnectionEvents(connected, disconnected);

	for (const uint32_t id : disconnected)
		_pendingDisconnects.push_back(ToPeerID(id));

	std::vector<PeerID> peers;
	for (const uint32_t id : connected)
		peers.push_back(ToPeerID(id));

	return peers;
}

std::vector<NetCode::PeerID> NetCode::UdpNetTransport::TakeDisconnectedPeers()
{
	std::vector<PeerID> peers;
	peers.swap(_pendingDisconnects);
	return peers;
}
