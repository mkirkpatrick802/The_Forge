#pragma once
#include "INetTransport.h"
#include "UdpTransport.h"

namespace NetCode
{
	// INetTransport over the dedicated-server UDP transport.
	//
	// Thin by design: UdpTransport already does framing, reliability, ordering and
	// fragmentation, so all this adds is the peer vocabulary NetworkManager speaks.
	// The server is SERVER_PEER_ID on both sides; clients are their connection ids.
	class UdpNetTransport final : public INetTransport
	{
	public:
		bool Start() override;
		void Shutdown() override;
		void Update() override;

		PeerID GetLocalPeerID() const override;
		bool IsAuthority() const override { return GetUdpTransport().IsServer(); }
		PeerID GetAuthorityPeerID() const override { return SERVER_PEER_ID; }
		bool IsSessionActive() const override;

		std::vector<PeerID> GetRemotePeers() const override;
		bool HasPeer(PeerID peer) const override;

		void Disconnect(PeerID peer) override;
		bool SendTo(PeerID peer, const OutputByteStream& stream, bool reliable) override;
		std::vector<NetMessage> Receive() override;

		std::vector<PeerID> TakeConnectedPeers() override;
		std::vector<PeerID> TakeDisconnectedPeers() override;

	private:
		// A client tags everything from the server as connection id 0; a server sees
		// real ids. This is the one place that difference is resolved.
		static PeerID ToPeerID(uint32_t connectionId);

		// Our role, from the command line rather than the live transport: a
		// disconnect is reported after Shutdown() has already cleared the mode.
		static bool IsClientRole();

	private:
		// TakeConnectionEvents drains both lists at once, so disconnects wait here
		// until TakeDisconnectedPeers() is called for them.
		std::vector<PeerID> _pendingDisconnects;
	};
}
