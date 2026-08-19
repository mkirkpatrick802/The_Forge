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

		bool IsDedicatedServer() const { return role == ENetRole::ENR_DedicatedServer; }
		bool IsClient() const { return role == ENetRole::ENR_Client; }

		// True for any role that talks to a dedicated server, i.e. not the Steam path.
		bool UsesDedicatedServerModel() const { return role != ENetRole::ENR_Standalone; }

		// Whether Steam should be initialised at all. The dedicated-server model has
		// its own transport, and editor/offline work has no reason to need a client.
		bool RequiresSteam() const { return !noSteam && !UsesDedicatedServerModel(); }
	};

	// Populates the process-wide options. Call once, first thing in main().
	void ParseLaunchOptions(int argc, char** argv);
	const LaunchOptions& GetLaunchOptions();
}
