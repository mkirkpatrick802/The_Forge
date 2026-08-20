#pragma once
#include <cstdint>
#include <string>

namespace Engine
{
	// How this process participates in a networked session.
	enum class ENetRole : uint8_t
	{
		ENR_Standalone,			// No dedicated-server networking; the existing Steam P2P path.
		ENR_DedicatedServer,	// Authoritative simulation with no local player.
		ENR_Client,				// Connects to a dedicated server.
	};

	// Parsed from the command line once at startup:
	//   --server             run as a dedicated server (headless unless --windowed)
	//   --connect <address>  run as a client against that server
	//   --port <number>      port to listen on / connect to (default 7777)
	//   --headless           no window and no renderer
	//   --windowed           force a window, even for --server
	//   --no-steam           skip Steam entirely (editor / offline authoring)
	//   --net-loss <percent> drop that %% of outgoing packets (transport testing)
	//   --net-selftest       run the transport's reliable-delivery self-test
	//   --insecure           dedicated server: accept players without Steam auth (dev only)
	//   --gslt <token>       dedicated server: Steam game server login token
	struct LaunchOptions
	{
		ENetRole role = ENetRole::ENR_Standalone;
		bool headless = false;
		bool noSteam = false;

		// Debug aids for the UDP transport. Loopback never loses a packet, so
		// simulated loss is the only way to exercise retransmission locally.
		uint32_t netLossPercent = 0;
		bool netSelfTest = false;
		std::string serverAddress = "127.0.0.1";
		uint16_t port = 7777;

		// --- dedicated server authentication ---
		// Accept players without proving who they are. Development and LAN only: it
		// means a client's claimed SteamID is taken at its word, and anyone can claim
		// any id. Opt-in rather than a fallback, so a server never quietly downgrades to
		// unverified because the Steam redistributable happened to be missing.
		bool insecure = false;

		// Game Server Login Token from Steam's partner site. Ties the server to the
		// appid and is what a publicly listed server needs; without one the server logs
		// on anonymously, which Steam accepts for LAN and development.
		std::string gsltToken;

		bool IsDedicatedServer() const { return role == ENetRole::ENR_DedicatedServer; }
		bool IsClient() const { return role == ENetRole::ENR_Client; }

		// True for any role that talks to a dedicated server, i.e. not the Steam path.
		bool UsesDedicatedServerModel() const { return role != ENetRole::ENR_Standalone; }

		// Whether Steam should be initialised at all.
		//
		// A *client* wants Steam up even on the dedicated-server model, because that is
		// where its identity comes from -- the SteamID and persona name a player is
		// known by. Its game traffic still goes over UDP; transport and identity are
		// separate concerns and only the transport is chosen by the role.
		//
		// A dedicated server does not: it has no local user to identify, and no Steam
		// client to ask. Validating the identities its clients assert is a job for the
		// Steam *game server* API, which is a separate dependency -- see PlayerIdentity.
		bool RequiresSteam() const { return !noSteam && !IsDedicatedServer(); }
	};

	// Populates the process-wide options. Call once, first thing in main().
	void ParseLaunchOptions(int argc, char** argv);
	const LaunchOptions& GetLaunchOptions();

	// Points this process at a server, after startup.
	//
	// Everything else in here is parsed once and then read-only, which is what makes it
	// safe to consult from anywhere. This is the deliberate exception: a main menu
	// decides which server to join at run time, and the role and address are exactly
	// what "join that one" means. Narrow on purpose -- a setter for the whole struct
	// would let anything rewrite the headless or Steam flags mid-run.
	//
	// Must be called before NetworkManager::StartNetCode, which is what reads them.
	void SetSessionTarget(ENetRole role, const std::string& address, uint16_t port);
}
