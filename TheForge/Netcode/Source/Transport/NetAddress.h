#pragma once
#include <cstdint>
#include <string>

namespace NetCode
{
	// An IPv4 endpoint. Stored in host byte order; conversion to network order
	// happens at the socket boundary so nothing above this layer has to think
	// about it.
	class NetAddress
	{
	public:
		NetAddress() = default;
		NetAddress(uint32_t address, uint16_t port) : _address(address), _port(port) {}

		// Resolves "127.0.0.1" or a hostname. Returns false if it cannot be resolved.
		static bool FromString(const std::string& host, uint16_t port, NetAddress& outAddress);

		uint32_t GetAddress() const { return _address; }
		uint16_t GetPort() const { return _port; }
		bool IsValid() const { return _address != 0 && _port != 0; }

		std::string ToString() const;

		bool operator==(const NetAddress& other) const { return _address == other._address && _port == other._port; }
		bool operator!=(const NetAddress& other) const { return !(*this == other); }

		// Lets an address be used as a map key without defining a hash.
		bool operator<(const NetAddress& other) const
		{
			if (_address != other._address) return _address < other._address;
			return _port < other._port;
		}

	private:
		uint32_t _address = 0;
		uint16_t _port = 0;
	};
}
