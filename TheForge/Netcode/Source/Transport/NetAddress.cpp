// Winsock must come before anything that may pull in windows.h.
#include <winsock2.h>
#include <ws2tcpip.h>

#include "NetAddress.h"

bool NetCode::NetAddress::FromString(const std::string& host, const uint16_t port, NetAddress& outAddress)
{
	// Try a plain dotted-quad first; it needs no name resolution and covers the
	// loopback case that local testing relies on.
	in_addr parsed{};
	if (inet_pton(AF_INET, host.c_str(), &parsed) == 1)
	{
		outAddress = NetAddress(ntohl(parsed.s_addr), port);
		return true;
	}

	addrinfo hints{};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	addrinfo* results = nullptr;
	if (getaddrinfo(host.c_str(), nullptr, &hints, &results) != 0 || results == nullptr)
		return false;

	const auto* addr = reinterpret_cast<sockaddr_in*>(results->ai_addr);
	outAddress = NetAddress(ntohl(addr->sin_addr.s_addr), port);

	freeaddrinfo(results);
	return true;
}

std::string NetCode::NetAddress::ToString() const
{
	char buffer[64];
	snprintf(buffer, sizeof(buffer), "%u.%u.%u.%u:%u",
		(_address >> 24) & 0xFF,
		(_address >> 16) & 0xFF,
		(_address >> 8) & 0xFF,
		_address & 0xFF,
		_port);

	return buffer;
}
