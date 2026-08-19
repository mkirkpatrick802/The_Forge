// Winsock must come before anything that may pull in windows.h.
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mstcpip.h>

// Present in mstcpip.h on current SDKs, defined here so an older one still builds.
#ifndef SIO_UDP_CONNRESET
#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR, 12)
#endif

#include "UdpSocket.h"

#include "Engine/System.h"

namespace
{
	constexpr uintptr_t INVALID_HANDLE = ~static_cast<uintptr_t>(0);

	// Winsock needs process-wide startup/shutdown. Reference counting it here keeps
	// that detail out of every caller, and out of engine startup.
	int g_socketCount = 0;

	bool AcquireWinsock()
	{
		if (g_socketCount > 0)
		{
			++g_socketCount;
			return true;
		}

		WSADATA data;
		if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
		{
			DEBUG_LOG("WSAStartup failed; no networking available.")
			return false;
		}

		g_socketCount = 1;
		return true;
	}

	void ReleaseWinsock()
	{
		if (g_socketCount <= 0) return;

		--g_socketCount;
		if (g_socketCount == 0)
			WSACleanup();
	}

	sockaddr_in ToSockAddr(const NetCode::NetAddress& address)
	{
		sockaddr_in out{};
		out.sin_family = AF_INET;
		out.sin_addr.s_addr = htonl(address.GetAddress());
		out.sin_port = htons(address.GetPort());
		return out;
	}
}

NetCode::UdpSocket::~UdpSocket()
{
	Close();
}

bool NetCode::UdpSocket::IsOpen() const
{
	return _handle != INVALID_HANDLE;
}

bool NetCode::UdpSocket::Open(const uint16_t port)
{
	if (IsOpen())
	{
		DEBUG_LOG("UdpSocket::Open called on an already open socket.")
		return false;
	}

	if (!AcquireWinsock())
		return false;

	const SOCKET handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (handle == INVALID_SOCKET)
	{
		DEBUG_LOG("Failed to create UDP socket (error %d).", WSAGetLastError())
		ReleaseWinsock();
		return false;
	}

	sockaddr_in local{};
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	local.sin_port = htons(port);

	if (bind(handle, reinterpret_cast<sockaddr*>(&local), sizeof(local)) == SOCKET_ERROR)
	{
		DEBUG_LOG("Failed to bind UDP socket to port %u (error %d).", port, WSAGetLastError())
		closesocket(handle);
		ReleaseWinsock();
		return false;
	}

	// Non-blocking: the transport is polled once per frame and must never stall
	// the game loop waiting for a packet.
	u_long nonBlocking = 1;
	if (ioctlsocket(handle, FIONBIO, &nonBlocking) == SOCKET_ERROR)
	{
		DEBUG_LOG("Failed to set UDP socket non-blocking (error %d).", WSAGetLastError())
		closesocket(handle);
		ReleaseWinsock();
		return false;
	}

	// Stop Winsock reporting ICMP port-unreachable as an error on this socket.
	//
	// UDP has no connections, but Windows will still fail a *recvfrom* with
	// WSAECONNRESET because an earlier sendto hit a closed port. On a server that is
	// routine -- any client that exits without a goodbye produces it -- and it makes
	// an unrelated peer's error surface while draining everyone else's packets.
	// Not fatal if it fails: ReceiveFrom handles the error either way.
	BOOL reportConnectionReset = FALSE;
	DWORD bytesReturned = 0;
	if (WSAIoctl(handle, SIO_UDP_CONNRESET, &reportConnectionReset, sizeof(reportConnectionReset),
		nullptr, 0, &bytesReturned, nullptr, nullptr) == SOCKET_ERROR)
	{
		DEBUG_LOG("Could not disable SIO_UDP_CONNRESET (error %d); relying on the ReceiveFrom path.", WSAGetLastError())
	}

	// Read back the actual port, which is what was wanted when 0 was passed.
	sockaddr_in bound{};
	int boundSize = sizeof(bound);
	if (getsockname(handle, reinterpret_cast<sockaddr*>(&bound), &boundSize) == 0)
		_boundPort = ntohs(bound.sin_port);
	else
		_boundPort = port;

	_handle = static_cast<uintptr_t>(handle);
	return true;
}

void NetCode::UdpSocket::Close()
{
	if (!IsOpen()) return;

	closesocket(static_cast<SOCKET>(_handle));
	_handle = INVALID_HANDLE;
	_boundPort = 0;

	ReleaseWinsock();
}

bool NetCode::UdpSocket::SendTo(const NetAddress& to, const void* data, const uint32_t size) const
{
	if (!IsOpen()) return false;

	const sockaddr_in destination = ToSockAddr(to);
	const int sent = sendto(static_cast<SOCKET>(_handle), static_cast<const char*>(data), static_cast<int>(size), 0,
		reinterpret_cast<const sockaddr*>(&destination), sizeof(destination));

	if (sent == SOCKET_ERROR)
	{
		DEBUG_LOG("sendto %s failed (error %d).", to.ToString().c_str(), WSAGetLastError())
		return false;
	}

	return static_cast<uint32_t>(sent) == size;
}

NetCode::UdpSocket::EReceiveResult NetCode::UdpSocket::ReceiveFrom(NetAddress& outFrom, void* buffer, const uint32_t bufferSize, uint32_t& outSize) const
{
	outSize = 0;
	if (!IsOpen()) return EReceiveResult::Empty;

	sockaddr_in from{};
	int fromSize = sizeof(from);

	const int received = recvfrom(static_cast<SOCKET>(_handle), static_cast<char*>(buffer), static_cast<int>(bufferSize), 0,
		reinterpret_cast<sockaddr*>(&from), &fromSize);

	if (received == SOCKET_ERROR)
	{
		const int error = WSAGetLastError();

		// Nothing pending is the normal case for a non-blocking socket.
		if (error == WSAEWOULDBLOCK) return EReceiveResult::Empty;

		// A previous sendto reached a closed port and Winsock is reporting the ICMP
		// back on the *receive* call. Skipped, not Empty: the error says nothing about
		// whether other peers have packets waiting, and treating it as an empty queue
		// abandoned the rest of the tick's traffic. One client being killed then
		// stalled every other client on the server, because their acks were among what
		// went unread and their send windows filled.
		//
		// Open() also asks Winsock to stop reporting this at all; this stays as the
		// belt to that braces, since the ioctl is not guaranteed on every stack.
		if (error == WSAECONNRESET) return EReceiveResult::Skipped;

		// A datagram larger than the buffer. Winsock has already discarded it, but the
		// rest of the queue is still fine.
		if (error == WSAEMSGSIZE)
		{
			DEBUG_LOG("Discarded an oversized datagram.")
			return EReceiveResult::Skipped;
		}

		DEBUG_LOG("recvfrom failed (error %d).", error)
		return EReceiveResult::Empty;
	}

	outFrom = NetAddress(ntohl(from.sin_addr.s_addr), ntohs(from.sin_port));
	outSize = static_cast<uint32_t>(received);
	return EReceiveResult::Received;
}
