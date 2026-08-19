#pragma once
#include <cstdint>
#include <vector>

#include "ByteStream.h"

namespace NetCode
{
	// Identifies one machine in a session. Steam uses the 64-bit SteamID; the UDP
	// transport widens its 32-bit connection id. Zero means "nobody".
	using PeerID = uint64_t;

	constexpr PeerID INVALID_PEER = 0;

	// The dedicated server's own peer id. Connection ids are handed out from 1 and
	// capped at MAX_CONNECTIONS, so UINT32_MAX can never collide with a client.
	constexpr PeerID SERVER_PEER_ID = 0xFFFFFFFFull;

	// How many peers the authority can hold replication state for at once. This is
	// the width of a GameObject's per-peer dirty mask, so it is a hard ceiling on
	// players in a match and cannot be raised past 32 without widening that mask.
	constexpr uint32_t MAX_REPLICATION_PEERS = 32;

	// One whole game message, already reassembled and in order.
	struct NetMessage
	{
		PeerID from = INVALID_PEER;
		InputByteStream stream;
	};

	// What NetworkManager needs from a transport, and nothing more.
	//
	// Deliberately *message-oriented*: one Send carries one complete game message
	// however many datagrams that takes, and Receive hands back complete messages
	// only. Framing, fragmentation and reassembly are each transport's own
	// business -- that is what let the splitting code leave NetworkManager.
	//
	// Implementations: SteamTransport (P2P through a lobby), UdpNetTransport
	// (dedicated server model), OfflineTransport (no session at all).
	class INetTransport
	{
	public:
		virtual ~INetTransport() = default;

		// Brings the transport up. Called once, before the gameplay loop.
		virtual bool Start() = 0;
		virtual void Shutdown() {}

		// Pumped once per frame, before Receive().
		virtual void Update() = 0;

		// --- identity ---
		virtual PeerID GetLocalPeerID() const = 0;

		// True when this machine simulates the world. Every "should I run game
		// logic" question in the engine funnels through here.
		virtual bool IsAuthority() const = 0;

		// The peer that simulates the world. Equals GetLocalPeerID() on an authority.
		virtual PeerID GetAuthorityPeerID() const = 0;

		// True once this machine can actually exchange messages with someone.
		virtual bool IsSessionActive() const = 0;

		// Every peer we can send to right now; never includes ourselves.
		virtual std::vector<PeerID> GetRemotePeers() const = 0;
		virtual bool HasPeer(PeerID peer) const = 0;

		// Drops a peer, with a goodbye so it learns why rather than timing out. The
		// disconnect is reported through TakeDisconnectedPeers like any other, so a
		// kick and a departure clean up through exactly one path.
		virtual void Disconnect(PeerID peer) {}

		// --- messages ---
		virtual bool SendTo(PeerID peer, const OutputByteStream& stream, bool reliable) = 0;

		// Sent to every remote peer. Not virtual: no transport has a cheaper way to
		// do it than looping, and this keeps implementations to one send path.
		void SendToAll(const OutputByteStream& stream, const bool reliable)
		{
			for (const PeerID peer : GetRemotePeers())
				SendTo(peer, stream, reliable);
		}

		bool SendToAuthority(const OutputByteStream& stream, const bool reliable)
		{
			if (IsAuthority()) return false;
			return SendTo(GetAuthorityPeerID(), stream, reliable);
		}

		// Complete messages that arrived since the last call.
		virtual std::vector<NetMessage> Receive() = 0;

		// --- connection events ---
		// Drained once per frame by NetworkManager, which turns them into spawns and
		// despawns. Reporting them rather than acting on them is what keeps the
		// transport ignorant of what a "player" is.
		virtual std::vector<PeerID> TakeConnectedPeers() = 0;
		virtual std::vector<PeerID> TakeDisconnectedPeers() = 0;
	};
}
