#include "NetActions.h"

#include <process.h>

#include "GamerServices.h"
#include "Engine/LaunchOptions.h"
#include "Engine/System.h"

std::unordered_map<NetCode::NetActionID, NetCode::NetActionRegistry::RequestHandler>& NetCode::NetActionRegistry::Requests()
{
	static std::unordered_map<NetActionID, RequestHandler> handlers;
	return handlers;
}

std::unordered_map<NetCode::NetActionID, NetCode::NetActionRegistry::EventHandler>& NetCode::NetActionRegistry::Events()
{
	static std::unordered_map<NetActionID, EventHandler> handlers;
	return handlers;
}

void NetCode::NetActionRegistry::RegisterRequest(const NetActionID id, RequestHandler handler)
{
	if (id == EngineRequest::Invalid)
	{
		DEBUG_LOG("NetActions: refusing to register a handler for request id 0.")
		return;
	}

	Requests()[id] = std::move(handler);
}

void NetCode::NetActionRegistry::RegisterEvent(const NetActionID id, EventHandler handler)
{
	if (id == EngineEvent::Invalid)
	{
		DEBUG_LOG("NetActions: refusing to register a handler for event id 0.")
		return;
	}

	Events()[id] = std::move(handler);
}

void NetCode::NetActionRegistry::UnregisterRequest(const NetActionID id)
{
	Requests().erase(id);
}

void NetCode::NetActionRegistry::UnregisterEvent(const NetActionID id)
{
	Events().erase(id);
}

bool NetCode::NetActionRegistry::DispatchRequest(const NetActionID id, const PeerID from, InputByteStream& payload)
{
	const auto it = Requests().find(id);
	if (it == Requests().end()) return false;

	it->second(from, payload);
	return true;
}

bool NetCode::NetActionRegistry::DispatchEvent(const NetActionID id, InputByteStream& payload)
{
	const auto it = Events().find(id);
	if (it == Events().end()) return false;

	it->second(payload);
	return true;
}

void NetCode::NetActionRegistry::Clear()
{
	Requests().clear();
	Events().clear();
}

/*
 *      Player identity
 */
void NetCode::PlayerIdentity::Write(OutputByteStream& stream) const
{
	stream.Write(id);
	stream.Write(name);

	stream.Write(static_cast<uint32_t>(authTicket.size()));
	if (!authTicket.empty())
		stream.WriteBytes(authTicket.data(), authTicket.size());
}

void NetCode::PlayerIdentity::Read(InputByteStream& stream)
{
	stream.Read(id);
	stream.Read(name);

	// Bounded on the way in, because this arrives from a client. A name is a display
	// string and nothing keys off it, so truncating is the right answer -- refusing
	// the whole join over a long name would be worse.
	if (name.size() > MAX_PLAYER_NAME_BYTES)
		name.resize(MAX_PLAYER_NAME_BYTES);

	uint32_t ticketBytes = 0;
	stream.Read(ticketBytes);

	// A ticket claiming to be larger than any real one is a client trying to make the
	// server allocate on its say-so. Dropped rather than clamped: a truncated ticket
	// would fail validation anyway, and this way the refusal is honest about why.
	if (ticketBytes == 0 || ticketBytes > MAX_TICKET_BYTES_ON_WIRE)
	{
		authTicket.clear();
		return;
	}

	authTicket.resize(ticketBytes);
	stream.ReadBytes(authTicket.data(), ticketBytes);
}

NetCode::PlayerIdentity NetCode::PlayerIdentity::Local()
{
	PlayerIdentity identity;

	// Steam is initialised for a client even when its traffic goes over UDP to a
	// dedicated server, precisely so this works -- the transport and the identity are
	// separate concerns. GamerServices only exists when a Steam client came up.
	if (Engine::REQUIRE_GAMER_SERVICES && GamerServices::instance != nullptr)
	{
		identity.id = GetGamerService().GetLocalPlayerID();
		identity.name = GetGamerService().GetLocalPlayerName();

		// Only attached once Steam has confirmed it. Sending the bytes the moment they
		// are handed over is the classic cause of a server seeing "auth ticket invalid"
		// from a perfectly legitimate player.
		if (GetGamerService().IsAuthTicketReady())
			identity.authTicket = GetGamerService().GetAuthTicket();

		if (identity.IsValid())
			return identity;
	}

	// No Steam: --no-steam, a headless test client, or an offline editor session.
	// Keyed on the process id so several clients from one folder are still distinct
	// players, which is exactly the case multi-client testing produces.
	const int processId = _getpid();
	identity.id = 0xDE0000000000ull | static_cast<uint64_t>(processId);
	identity.name = "Dev-" + std::to_string(processId);

	return identity;
}
