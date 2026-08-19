#pragma once
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace NetCode
{
	// Proves who a connecting player is, using Steam's game server authentication.
	//
	// The problem this solves: a client tells the server its SteamID, and on its own
	// that is a *claim*. Anyone can send any number. Steam's answer is a signed ticket
	// the client fetches from its own Steam client and the server hands to Steam's
	// backend for validation, which is the only party in a position to say whether the
	// SteamID and the person behind it actually match.
	//
	// The exchange:
	//
	//   1. client  GetAuthSessionTicket -> an opaque blob
	//   2. client  sends { claimed SteamID, name, ticket } in its Join request
	//   3. server  BeginAuthSession( ticket, claimed SteamID )
	//   4. Steam   ValidateAuthTicketResponse_t, asynchronously
	//   5. server  admits or kicks
	//
	// Step 4 is the point of the whole thing and it is **asynchronous**, which is why a
	// player is not admitted -- not spawned, not sent the world -- until it arrives.
	//
	// Validation also covers things a bare id never could: whether the account owns the
	// app, whether the ticket has been cancelled, and whether the account is VAC or
	// publisher banned. Those arrive through the same response.
	class SteamGameServerAuth
	{
	public:
		static SteamGameServerAuth& Get();

		SteamGameServerAuth();
		~SteamGameServerAuth();

		// Brings the game server up and logs it in. `gsltToken` is a Game Server Login
		// Token from Steam's partner site: required for a server that should be publicly
		// visible and tied to your appid, and optional for LAN and development, where an
		// anonymous logon is accepted.
		//
		// Returns false if the Steamworks redistributable is not present or the logon
		// could not be started. A server that wants authentication and gets false back
		// should refuse to run rather than quietly accept unverified players.
		bool Start(uint16_t gamePort, uint16_t queryPort, const std::string& gsltToken);

		void Shutdown();

		// Pumped once per frame. Game server callbacks are a separate queue from the
		// client ones, so this is not the same call GamerServices makes.
		void Update() const;

		bool IsRunning() const { return _running; }

		// True once Steam has accepted the logon. Tickets cannot validate before this,
		// so a player arriving in the gap is held rather than refused.
		bool IsLoggedOn() const;

		// Hands a ticket to Steam. False means the ticket was malformed or refused
		// outright -- a straightforward reject. True means *pending*: the verdict
		// arrives later through onValidated.
		bool BeginAuth(uint64_t claimedSteamID, const std::vector<uint8_t>& ticket);

		// Stops tracking a player. Must be called for every successful BeginAuth, or
		// Steam keeps believing the player is on this server.
		void EndAuth(uint64_t steamID);

		// The verdict. `reason` is nullptr when ok.
		std::function<void(uint64_t steamID, bool ok, const char* reason)> onValidated;

	private:
		struct Impl;
		std::unique_ptr<Impl> _impl;
		bool _running = false;
	};

	// A Steam auth ticket is a few hundred bytes; the cap is what the server will accept
	// off the wire, and exists so a join message cannot be made arbitrarily large.
	constexpr uint32_t MAX_AUTH_TICKET_BYTES = 2048;
}
