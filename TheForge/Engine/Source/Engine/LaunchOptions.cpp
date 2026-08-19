#include "LaunchOptions.h"

#include <cstdlib>
#include <cstring>
#include <iostream>

namespace
{
	Engine::LaunchOptions g_launchOptions;
}

void Engine::ParseLaunchOptions(const int argc, char** argv)
{
	LaunchOptions options;
	bool windowedRequested = false;

	for (int i = 1; i < argc; ++i)
	{
		const char* arg = argv[i];
		const bool hasValue = i + 1 < argc;

		if (std::strcmp(arg, "--server") == 0)
		{
			options.role = ENetRole::ENR_DedicatedServer;

			// A dedicated server is headless by default; --windowed opts back out.
			options.headless = true;
		}
		else if (std::strcmp(arg, "--connect") == 0 && hasValue)
		{
			options.role = ENetRole::ENR_Client;
			options.serverAddress = argv[++i];
		}
		else if (std::strcmp(arg, "--port") == 0 && hasValue)
		{
			options.port = static_cast<uint16_t>(std::atoi(argv[++i]));
		}
		else if (std::strcmp(arg, "--headless") == 0)
		{
			options.headless = true;
		}
		else if (std::strcmp(arg, "--no-steam") == 0)
		{
			options.noSteam = true;
		}
		else if (std::strcmp(arg, "--net-loss") == 0 && hasValue)
		{
			const int percent = std::atoi(argv[++i]);
			options.netLossPercent = static_cast<uint32_t>(percent < 0 ? 0 : (percent > 100 ? 100 : percent));
		}
		else if (std::strcmp(arg, "--net-selftest") == 0)
		{
			options.netSelfTest = true;
		}
		else if (std::strcmp(arg, "--insecure") == 0)
		{
			options.insecure = true;
		}
		else if (std::strcmp(arg, "--gslt") == 0 && hasValue)
		{
			options.gsltToken = argv[++i];
		}
		else if (std::strcmp(arg, "--windowed") == 0)
		{
			windowedRequested = true;
		}
		else
		{
			std::cerr << "Unknown launch argument: " << arg << '\n';
		}
	}

	// Applied last so it wins regardless of argument order.
	if (windowedRequested)
		options.headless = false;

	g_launchOptions = options;
}

const Engine::LaunchOptions& Engine::GetLaunchOptions()
{
	return g_launchOptions;
}
