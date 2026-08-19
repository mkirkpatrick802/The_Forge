#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "ByteStream.h"
#include "INetTransport.h"

namespace NetCode
{
	// Named actions on the wire, alongside world state and input.
	//
	// The distinction that matters is between the two directions, and it is not
	// symmetric:
	//
	//   A *request* travels client -> authority and is a **petition**. The server
	//   validates it and may refuse. Nothing in its payload is trustworthy: the
	//   sender is whoever the transport says it is, never whoever the payload
	//   claims to be.
	//
	//   An *event* travels authority -> client and is a **statement of fact**. A
	//   client applies it; it does not get to argue.
	//
	// Both go on the reliable channel. Neither is superseded by the next one the way
	// input is, so a dropped one is simply lost -- see Client Input for why that
	// distinction decides the channel.
	using NetActionID = uint16_t;

	// The engine owns the low ids; a game defines its own from GAME_ACTION_BASE up.
	// Netcode therefore never needs to know what a ship piece or a team is, and a
	// game can add actions without touching this module.
	constexpr NetActionID GAME_ACTION_BASE = 256;

	// Wire values. Never renumber or reorder these -- an old client and a new server
	// disagreeing about what id 3 means is silent misbehaviour, not a clean failure.
	namespace EngineRequest
	{
		enum : NetActionID
		{
			Invalid = 0,

			// First thing a client sends once the server has welcomed it. Carries who
			// the player claims to be; see PlayerIdentity for what that is worth.
			Join = 1,
		};
	}

	namespace EngineEvent
	{
		enum : NetActionID
		{
			Invalid = 0,

			// The authority has recorded this player's identity and assigned them a
			// slot. Sent once, in reply to Join.
			JoinAccepted = 1,

			// A request could not be honoured, with a human-readable reason. Worth
			// having as a first-class event: without it a refused action is
			// indistinguishable from a dropped one, from the client's side.
			RequestDenied = 2,
		};
	}

	// Where handlers for the above are registered.
	//
	// Deliberately not a compile-time table: the engine registers its own handlers,
	// the game registers its own, and neither has to know the other's ids exist.
	class NetActionRegistry
	{
	public:
		// Runs on the authority when a client asks for something. `from` is the
		// transport's identification of the sender -- resolve the acting player from
		// it and nothing else.
		using RequestHandler = std::function<void(PeerID from, InputByteStream& payload)>;

		// Runs on a client when the authority states something happened.
		using EventHandler = std::function<void(InputByteStream& payload)>;

		static void RegisterRequest(NetActionID id, RequestHandler handler);
		static void RegisterEvent(NetActionID id, EventHandler handler);

		static void UnregisterRequest(NetActionID id);
		static void UnregisterEvent(NetActionID id);

		// Returns false when nothing is registered for the id, which is a real
		// possibility across builds rather than a programming error -- the caller
		// logs it and drops the message rather than asserting.
		static bool DispatchRequest(NetActionID id, PeerID from, InputByteStream& payload);
		static bool DispatchEvent(NetActionID id, InputByteStream& payload);

		static void Clear();

	private:
		static std::unordered_map<NetActionID, RequestHandler>& Requests();
		static std::unordered_map<NetActionID, EventHandler>& Events();
	};

	// Who a player is, as far as the game is concerned.
	//
	// The id and name are the SteamID and persona name, read by the client from its own
	// Steam client. On their own those are a *claim* -- anyone can send any number --
	// so an authenticating server does not believe them: it validates the ticket that
	// travels with them through the Steam game server API, and Steam's backend is the
	// only party in a position to say whether the id and the person behind it match.
	//
	// See SteamGameServerAuth for the exchange, and NetworkManager::HandleJoinRequest
	// for the point at which a claim becomes a fact. A server started with --insecure
	// skips all of it and takes the claim at its word, which is why that flag is
	// explicit rather than a fallback.
	struct PlayerIdentity
	{
		uint64_t id = 0;
		std::string name;

		// Steam's proof that the id above really is this player. Empty on a server
		// running --insecure, and on a client with no Steam -- which is why an
		// authenticating server refuses an empty one rather than treating it as
		// "nothing to check".
		std::vector<uint8_t> authTicket;

		bool IsValid() const { return id != 0; }

		void Write(OutputByteStream& stream) const;
		void Read(InputByteStream& stream);

		// The local player's identity: Steam's if a Steam client is up, otherwise a
		// per-process development identity so an offline or headless build still has
		// something stable to key a player on.
		static PlayerIdentity Local();
	};

	// The longest a ticket may be on the wire, mirroring MAX_AUTH_TICKET_BYTES. A
	// client controls this, so it needs a bound.
	constexpr uint32_t MAX_TICKET_BYTES_ON_WIRE = 2048;

	// The longest a name may be on the wire. A client controls this string, so it
	// needs a bound -- without one a join message can be made arbitrarily large.
	constexpr uint32_t MAX_PLAYER_NAME_BYTES = 64;
}
