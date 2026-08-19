#include "ServerConsole.h"

#include <iostream>

#include "CommandRegistry.h"
#include "System.h"

Engine::ServerConsole::ServerConsole()
	: _shared(std::make_shared<Shared>())
{
	DEBUG_LOG("Console ready. Type 'help' for commands.")

	// The shared_ptr is captured by value, so the state outlives this object if the
	// thread is still parked in a read when the server shuts down.
	_reader = std::thread([shared = _shared]
	{
		std::string line;
		while (shared->running && std::getline(std::cin, line))
		{
			if (line.empty()) continue;

			std::lock_guard lock(shared->mutex);
			shared->pending.push(line);
		}
	});
}

Engine::ServerConsole::~ServerConsole()
{
	_shared->running = false;

	// Detached rather than joined: getline is blocking on a handle nothing here can
	// close portably, so joining would hang shutdown until somebody pressed return.
	if (_reader.joinable())
		_reader.detach();
}

void Engine::ServerConsole::Update()
{
	std::queue<std::string> commands;
	{
		std::lock_guard lock(_shared->mutex);
		std::swap(commands, _shared->pending);
	}

	while (!commands.empty())
	{
		const std::string command = commands.front();
		commands.pop();

		if (command == "help")
		{
			DEBUG_LOG("Commands are registered by the engine and the running game mode.")
			DEBUG_LOG("  startmatch   force the match to begin")
			DEBUG_LOG("  teams        report the current match state and team sizes")
			continue;
		}

		if (!CommandRegistry::ExecuteCommand(command))
			DEBUG_LOG("Unknown command: %s", command.c_str())
	}
}
