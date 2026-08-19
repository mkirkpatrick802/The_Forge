#pragma once
#include <cstdint>

#include "NetAddress.h"

namespace NetCode
{
	// A non-blocking UDP socket.
	//
	// The platform socket handle is kept as an opaque integer so that <winsock2.h>
	// stays out of every translation unit that needs to send a packet -- it has to
	// be included before windows.h, and Engine/System.h pulls windows.h in.
	class UdpSocket
	{
	public:
		UdpSocket() = default;
		~UdpSocket();

		UdpSocket(const UdpSocket&) = delete;
		UdpSocket& operator=(const UdpSocket&) = delete;

		// Binds to a local port. Pass 0 for an OS-assigned one, which is what a
		// client wants. Returns false and logs on failure.
		bool Open(uint16_t port);
		void Close();
		bool IsOpen() const;

		// The port actually bound, which matters when 0 was requested.
		uint16_t GetBoundPort() const { return _boundPort; }

		bool SendTo(const NetAddress& to, const void* data, uint32_t size) const;

		// Outcome of one recvfrom. A datagram that failed is deliberately a different
		// answer from an empty queue: they call for opposite responses, and conflating
		// them meant one bad datagram abandoned every other peer's traffic for the
		// frame -- see the note on WSAECONNRESET in ReceiveFrom.
		enum class EReceiveResult : uint8_t
		{
			Empty,		// nothing pending; stop draining this tick
			Received,	// outSize bytes are in the buffer
			Skipped,	// this datagram is gone, but the queue may not be; keep draining
		};

		// Never blocks. Datagrams larger than bufferSize are discarded rather than
		// silently truncated, and reported as Skipped.
		EReceiveResult ReceiveFrom(NetAddress& outFrom, void* buffer, uint32_t bufferSize, uint32_t& outSize) const;

	private:
		uintptr_t _handle = ~static_cast<uintptr_t>(0);
		uint16_t _boundPort = 0;
	};
}
