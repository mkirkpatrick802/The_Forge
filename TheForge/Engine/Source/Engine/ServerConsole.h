#pragma once
#include <atomic>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

namespace Engine
{
	// Lets an operator type commands at a headless server.
	//
	// A dedicated server has no window, no ImGui and therefore no chat terminal, so the
	// CommandRegistry it shares with the editor is unreachable on it -- which leaves
	// things like "start the match" with no way to be invoked at all.
	//
	// Reading stdin blocks and the gameplay loop must not, so the read happens on its
	// own thread. Commands are *executed* on the main thread when Update drains the
	// queue: they touch the level and the game mode, and neither is safe to poke from
	// another thread.
	class ServerConsole
	{
	public:
		ServerConsole();
		~ServerConsole();

		// Runs whatever has been typed since the last call. Main thread only.
		void Update();

	private:
		// Held by both sides through a shared_ptr, because the reader thread parks in a
		// blocking read that nothing can portably interrupt: it is detached at shutdown
		// and must not be left pointing at a destroyed queue.
		struct Shared
		{
			std::mutex mutex;
			std::queue<std::string> pending;
			std::atomic<bool> running{true};
		};

		std::shared_ptr<Shared> _shared;
		std::thread _reader;
	};
}
