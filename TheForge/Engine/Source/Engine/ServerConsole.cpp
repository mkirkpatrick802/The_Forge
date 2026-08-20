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

		// Typed at without a slash, but commands are registered with one. ExecuteCommand
		// accepts either form, so "startmatch" and "/startmatch" both work here and the
		// operator does not have to know the convention.
		if (!CommandRegistry::ExecuteCommand(command))
			DEBUG_LOG("Unknown command: %s  (type help)", command.c_str())
	}
}
