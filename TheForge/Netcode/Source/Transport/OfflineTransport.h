#pragma once
#include "INetTransport.h"

namespace NetCode
{
	// No session, no peers, no wire. Used by the editor and any --no-steam run.
	//
	// It exists so NetworkManager never has to test for a null transport: offline
	// is just the case where every send goes nowhere and nothing ever arrives. It
	// is the authority, which is what keeps single-player simulating normally.
	class OfflineTransport final : public INetTransport
	{
	public:
		bool Start() override { return true; }
		void Update() override {}

		PeerID GetLocalPeerID() const override { return LOCAL_PLAYER; }
		bool IsAuthority() const override { return true; }
		PeerID GetAuthorityPeerID() const override { return LOCAL_PLAYER; }
		bool IsSessionActive() const override { return false; }

		std::vector<PeerID> GetRemotePeers() const override { return {}; }
		bool HasPeer(const PeerID peer) const override { return peer == LOCAL_PLAYER; }

		bool SendTo(PeerID, const OutputByteStream&, bool) override { return false; }
		std::vector<NetMessage> Receive() override { return {}; }

		std::vector<PeerID> TakeConnectedPeers() override { return {}; }
		std::vector<PeerID> TakeDisconnectedPeers() override { return {}; }

	private:
		// A non-zero id, so the local pawn's _controllingPlayer matches it and
		// PlayerController::IsLocalPlayer() finds a camera. Zero is "unassigned".
		static constexpr PeerID LOCAL_PLAYER = 1;
	};
}
