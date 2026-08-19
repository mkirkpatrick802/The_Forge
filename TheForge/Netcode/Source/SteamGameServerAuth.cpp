#include "SteamGameServerAuth.h"

#include "Engine/System.h"
#include "steam/steam_api.h"
#include "steam/steam_gameserver.h"

namespace
{
	// Reported to the server browser and used by Steam to match client and server
	// builds. Bump it when the wire format changes in a way that makes old clients
	// incompatible -- it is the same reasoning as TRANSPORT_PROTOCOL_ID, one level up.
	constexpr const char* SERVER_VERSION = "1.0.0.0";
	constexpr const char* SERVER_PRODUCT = "ProjectSteel";
	constexpr const char* SERVER_DESCRIPTION = "Project Steel";
	constexpr const char* SERVER_MOD_DIR = "projectsteel";

	const char* AuthResponseText(const EAuthSessionResponse response)
	{
		switch (response)
		{
		case k_EAuthSessionResponseOK:                        return nullptr;
		case k_EAuthSessionResponseUserNotConnectedToSteam:    return "not connected to Steam";
		case k_EAuthSessionResponseNoLicenseOrExpired:         return "does not own the game";
		case k_EAuthSessionResponseVACBanned:                  return "VAC banned";
		case k_EAuthSessionResponseLoggedInElseWhere:          return "logged in elsewhere";
		case k_EAuthSessionResponseVACCheckTimedOut:           return "VAC check timed out";
		case k_EAuthSessionResponseAuthTicketCanceled:         return "auth ticket cancelled";
		case k_EAuthSessionResponseAuthTicketInvalidAlreadyUsed: return "auth ticket already used";
		case k_EAuthSessionResponseAuthTicketInvalid:          return "auth ticket invalid";
		case k_EAuthSessionResponsePublisherIssuedBan:         return "banned by the publisher";
		default:                                               return "rejected by Steam";
		}
	}

	const char* BeginResultText(const EBeginAuthSessionResult result)
	{
		switch (result)
		{
		case k_EBeginAuthSessionResultOK:              return nullptr;
		case k_EBeginAuthSessionResultInvalidTicket:   return "malformed auth ticket";
		case k_EBeginAuthSessionResultDuplicateRequest:return "already authenticating";
		case k_EBeginAuthSessionResultInvalidVersion:  return "auth ticket from a different Steam version";
		case k_EBeginAuthSessionResultGameMismatch:    return "auth ticket is for a different game";
		case k_EBeginAuthSessionResultExpiredTicket:   return "expired auth ticket";
		default:                                       return "auth ticket refused";
		}
	}
}

struct NetCode::SteamGameServerAuth::Impl
{
	explicit Impl(SteamGameServerAuth& owner)
		: validateCallback(this, &Impl::OnValidateAuthTicketResponse)
		, _owner(owner)
	{
	}

	// The game server callback queue is separate from the client one, hence the
	// GAMESERVER variant of the macro. Registering this on the client queue would
	// silently never fire.
	STEAM_GAMESERVER_CALLBACK(Impl, OnValidateAuthTicketResponse, ValidateAuthTicketResponse_t, validateCallback);

	SteamGameServerAuth& _owner;
};

void NetCode::SteamGameServerAuth::Impl::OnValidateAuthTicketResponse(ValidateAuthTicketResponse_t* response)
{
	const uint64_t steamID = response->m_SteamID.ConvertToUint64();
	const char* reason = AuthResponseText(response->m_eAuthSessionResponse);

	if (reason == nullptr)
		DEBUG_LOG("Auth: Steam verified %llu.", steamID)
	else
		DEBUG_LOG("Auth: Steam rejected %llu -- %s.", steamID, reason)

	if (_owner.onValidated)
		_owner.onValidated(steamID, reason == nullptr, reason);
}

NetCode::SteamGameServerAuth& NetCode::SteamGameServerAuth::Get()
{
	static SteamGameServerAuth instance;
	return instance;
}

NetCode::SteamGameServerAuth::SteamGameServerAuth() = default;

NetCode::SteamGameServerAuth::~SteamGameServerAuth()
{
	Shutdown();
}

bool NetCode::SteamGameServerAuth::Start(const uint16_t gamePort, const uint16_t queryPort, const std::string& gsltToken)
{
	if (_running) return true;

	// eServerModeAuthentication rather than ...AndSecure: the secure variant turns on
	// VAC for connected clients, which is a decision with its own consequences (and its
	// own support burden) and should be made deliberately rather than inherited from a
	// default. Ticket validation, ownership and ban checks all work either way.
	SteamErrMsg error = {};
	if (SteamGameServer_InitEx(0, gamePort, queryPort, eServerModeAuthentication, SERVER_VERSION, &error) != k_ESteamAPIInitResult_OK)
	{
		DEBUG_LOG("Auth: could not start the Steam game server -- %s", error)
		DEBUG_LOG("Auth: the machine needs the Steamworks redistributable (SteamCMD app 1007) or a running Steam client.")
		return false;
	}

	if (SteamGameServer() == nullptr)
	{
		DEBUG_LOG("Auth: Steam game server interface unavailable after init.")
		SteamGameServer_Shutdown();
		return false;
	}

	_impl = std::make_unique<Impl>(*this);

	SteamGameServer()->SetProduct(SERVER_PRODUCT);
	SteamGameServer()->SetGameDescription(SERVER_DESCRIPTION);
	SteamGameServer()->SetModDir(SERVER_MOD_DIR);
	SteamGameServer()->SetDedicatedServer(true);

	// A token ties the server to your appid and is what a publicly listed server needs.
	// Anonymous logon is accepted for LAN and development, and is why this works on a
	// machine that has never seen a token.
	if (!gsltToken.empty())
	{
		DEBUG_LOG("Auth: logging on with a game server login token.")
		SteamGameServer()->LogOn(gsltToken.c_str());
	}
	else
	{
		DEBUG_LOG("Auth: no login token given, logging on anonymously (fine for LAN and development).")
		SteamGameServer()->LogOnAnonymous();
	}

	_running = true;
	return true;
}

void NetCode::SteamGameServerAuth::Shutdown()
{
	if (!_running) return;

	_running = false;
	_impl.reset();

	if (SteamGameServer() != nullptr)
		SteamGameServer()->LogOff();

	SteamGameServer_Shutdown();
}

void NetCode::SteamGameServerAuth::Update() const
{
	if (!_running) return;

	// Without this the validation response never arrives and every player sits pending
	// until their auth window expires.
	SteamGameServer_RunCallbacks();
}

bool NetCode::SteamGameServerAuth::IsLoggedOn() const
{
	return _running && SteamGameServer() != nullptr && SteamGameServer()->BLoggedOn();
}

bool NetCode::SteamGameServerAuth::BeginAuth(const uint64_t claimedSteamID, const std::vector<uint8_t>& ticket)
{
	if (!_running || SteamGameServer() == nullptr) return false;
	if (ticket.empty() || ticket.size() > MAX_AUTH_TICKET_BYTES) return false;

	// The claimed id is passed in deliberately: Steam checks the ticket *against* it, so
	// a ticket belonging to somebody else fails here rather than being accepted under
	// whichever name came with it.
	const EBeginAuthSessionResult result = SteamGameServer()->BeginAuthSession(
		ticket.data(), static_cast<int>(ticket.size()), CSteamID(claimedSteamID));

	if (const char* reason = BeginResultText(result))
	{
		DEBUG_LOG("Auth: refused %llu outright -- %s.", claimedSteamID, reason)
		return false;
	}

	return true;
}

void NetCode::SteamGameServerAuth::EndAuth(const uint64_t steamID)
{
	if (!_running || SteamGameServer() == nullptr) return;

	SteamGameServer()->EndAuthSession(CSteamID(steamID));
}
