#include "SteamTransport.h"

#include <ranges>

#include "GamerServices.h"
#include "NetworkManager.h"
#include "Engine/System.h"

namespace
{
	// Set by NetworkManager while the Steam transport is the live one, so the
	// Steam callbacks have something to call into. Null on every other path.
	NetCode::SteamTransport* g_steamTransport = nullptr;
}

NetCode::SteamTransport* NetCode::GetSteamTransport()
{
	return g_steamTransport;
}

NetCode::SteamTransport::SteamTransport()
{
	// Identity is Steam's to hand out. Reaching for it is safe here and only here:
	// this transport is constructed only when Steam was actually initialised.
	_localUserID = GetGamerService().GetLocalPlayerID();
	g_steamTransport = this;
}

bool NetCode::SteamTransport::Start()
{
	// Steam has no address to dial -- joining means finding a lobby.
	GetGamerService().LobbySearchAsync();
	return true;
}

void NetCode::SteamTransport::Shutdown()
{
	GetGamerService().LeaveLobby(_lobbyID);
	_lobbyID = 0;
	g_steamTransport = nullptr;
}

void NetCode::SteamTransport::Update()
{
	// Pumps Steam's callbacks, which is what drives EnterLobby and the peer events.
	// Steam client callbacks are pumped by NetworkManager::Update for every transport,
	// not just this one -- a UDP client still needs them for its auth ticket.
}

std::vector<NetCode::PeerID> NetCode::SteamTransport::GetRemotePeers() const
{
	std::vector<PeerID> peers;
	for (const PeerID peer : _playerNames | std::views::keys)
	{
		if (peer == _localUserID) continue;
		peers.push_back(peer);
	}

	return peers;
}

bool NetCode::SteamTransport::SendTo(const PeerID peer, const OutputByteStream& stream, bool)
{
	// Steam P2P offers no unreliable mode here, so every message is reliable. The
	// flag is honoured by the UDP transport, where it actually costs something.
	if (peer == _localUserID || peer == INVALID_PEER) return false;
	return GetGamerService().SendP2PReliable(stream, peer);
}

std::vector<NetCode::NetMessage> NetCode::SteamTransport::Receive()
{
	std::vector<NetMessage> messages;

	const uint32_t packetSizeBytes = MAX_PACKET_SIZE_BYTES;
	uint32_t incomingSize = 0;
	InputByteStream stream(packetSizeBytes);
	uint64_t fromPlayer;

	// Bounded per call so a flood cannot stall a frame indefinitely; whatever is
	// left stays in Steam's queue and is picked up next time round.
	int receivedPacketCount = 0;
	while (GetGamerService().IsP2PPacketAvailable(incomingSize) && receivedPacketCount < 10)
	{
		if (incomingSize > packetSizeBytes)
		{
			// Consume and discard. Skipping it without reading would leave it at the
			// front of Steam's queue, so IsP2PPacketAvailable would keep reporting it
			// and this loop would never terminate.
			GetGamerService().ReadP2PPacket(stream.GetBuffer(), packetSizeBytes, fromPlayer);
			++receivedPacketCount;
			DEBUG_LOG("Dropped oversized packet of %u bytes from %llu", incomingSize, fromPlayer)
			continue;
		}

		const uint32_t readByteCount = GetGamerService().ReadP2PPacket(stream.GetBuffer(), packetSizeBytes, fromPlayer);
		if (readByteCount == 0)
		{
			// Nothing consumed, so the queue hasn't advanced -- bail out rather than
			// spin on the same packet for the rest of the frame.
			break;
		}

		stream.ResetToCapacity(readByteCount);
		++receivedPacketCount;

		uint8_t packetState;
		stream.Read(packetState); // Chunk marker written by SendP2PReliable.

		if (packetState == 0) // Standalone packet
		{
			messages.push_back({ fromPlayer, stream });
			continue;
		}

		auto& buffer = _reassemblyBuffer[fromPlayer];
		if (packetState == 1) // First packet of a split sequence
			buffer.clear(); // Start fresh for this sender

		// Append the packet's content (excluding the marker byte) to the buffer.
		const auto bytes = reinterpret_cast<const uint8_t*>(stream.GetBuffer());
		buffer.insert(buffer.end(), bytes + 1, bytes + readByteCount);

		if (packetState == 3) // Last packet of a split sequence
		{
			messages.push_back({ fromPlayer, InputByteStream(buffer.data(), static_cast<uint32_t>(buffer.size())) });
			_reassemblyBuffer.erase(fromPlayer);
		}
	}

	return messages;
}

std::vector<NetCode::PeerID> NetCode::SteamTransport::TakeConnectedPeers()
{
	std::vector<PeerID> peers;
	peers.swap(_connectedPeers);
	return peers;
}

std::vector<NetCode::PeerID> NetCode::SteamTransport::TakeDisconnectedPeers()
{
	std::vector<PeerID> peers;
	peers.swap(_disconnectedPeers);
	return peers;
}

void NetCode::SteamTransport::EnterLobby(const uint64_t lobbyID)
{
	_lobbyID = lobbyID;
	UpdateLobbyPlayers();

	// The host is a player too, and nothing else will report it as having joined.
	// A dedicated server never reaches this path, which is exactly why it ends up
	// with no local pawn.
	if (_isOwner)
		_connectedPeers.push_back(_localUserID);
}

void NetCode::SteamTransport::UpdateLobbyPlayers()
{
	_playerCount = GetGamerService().GetLobbyNumPlayers(_lobbyID);
	_ownerID = GetGamerService().GetOwnerID(_lobbyID);

	if (_ownerID == _localUserID)
		_isOwner = true;

	GetGamerService().GetLobbyPlayerMap(_lobbyID, _playerNames);
	DEBUG_LOG("Current player count: %d", _playerCount)
}

void NetCode::SteamTransport::OnPeerEntered(const PeerID peer)
{
	// Only the authority onboards anyone; a client learns about the new player
	// from the world state the host replicates.
	if (!_isOwner) return;
	_connectedPeers.push_back(peer);
}

void NetCode::SteamTransport::OnPeerLeft(const PeerID peer)
{
	if (!_playerNames.contains(peer)) return;

	_playerNames.erase(peer);
	_playerCount--;
	_disconnectedPeers.push_back(peer);
}
