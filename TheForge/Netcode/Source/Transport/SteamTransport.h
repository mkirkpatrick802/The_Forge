#pragma once
#include <map>
#include <string>
#include <unordered_map>

#include "INetTransport.h"

namespace NetCode
{
	// INetTransport over Steam lobbies and P2P.
	//
	// This is where the lobby now lives. NetworkManager used to own the member
	// list, the owner id and the reassembly buffer, which meant every one of those
	// Steam concepts leaked into game logic; they are all Steam's framing, so they
	// belong on this side of the interface.
	//
	// The Steam callbacks in GamerServiceSteam.cpp drive the lobby entry points
	// below, and the resulting joins and leaves surface as connection events.
	class SteamTransport final : public INetTransport
	{
	public:
		SteamTransport();

		bool Start() override;
		void Shutdown() override;
		void Update() override;

		PeerID GetLocalPeerID() const override { return _localUserID; }

		// The lobby owner simulates. Solo counts as authority too, so a lobby of one
		// -- and the moment before anyone else joins -- still runs its own game.
		bool IsAuthority() const override { return _isOwner || _playerCount <= 1; }
		PeerID GetAuthorityPeerID() const override { return _ownerID; }
		bool IsSessionActive() const override { return _lobbyID != 0; }

		std::vector<PeerID> GetRemotePeers() const override;
		bool HasPeer(PeerID peer) const override { return _playerNames.contains(peer); }

		bool SendTo(PeerID peer, const OutputByteStream& stream, bool reliable) override;
		std::vector<NetMessage> Receive() override;

		std::vector<PeerID> TakeConnectedPeers() override;
		std::vector<PeerID> TakeDisconnectedPeers() override;

		// --- driven by the Steam callbacks ---
		void EnterLobby(uint64_t lobbyID);
		void UpdateLobbyPlayers();
		void OnPeerEntered(PeerID peer);
		void OnPeerLeft(PeerID peer);

		bool GetIsOwner() const { return _isOwner; }
		int GetPlayerCount() const { return _playerCount; }

	private:
		uint64_t _lobbyID = 0;
		PeerID _ownerID = INVALID_PEER;
		PeerID _localUserID = INVALID_PEER;

		int _playerCount = 0;
		bool _isOwner = false;
		std::map<PeerID, std::string> _playerNames;

		// Steam P2P packets are capped, so a large message goes out in numbered
		// chunks and is accumulated here per sender until the last one lands.
		std::unordered_map<PeerID, std::vector<uint8_t>> _reassemblyBuffer;

		std::vector<PeerID> _connectedPeers;
		std::vector<PeerID> _disconnectedPeers;
	};

	// Valid only on the Steam path; see NetworkManager for which transport is live.
	SteamTransport* GetSteamTransport();
}
