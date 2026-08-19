#pragma once
#include <cstdint>

namespace NetCode
{
	// Every datagram starts with this magic number. It is not security -- it just
	// means stray traffic on the port, or a mismatched build, is dropped instead of
	// being parsed as a packet. Bump it whenever the wire format changes.
	constexpr uint32_t TRANSPORT_PROTOCOL_ID = 0x46524731; // 'FRG1'

	enum ETransportPacketType : uint8_t
	{
		ETP_ConnectRequest = 0,	// client -> server, repeated until answered
		ETP_ConnectAccept,		// server -> client, carries the assigned connection id
		ETP_ConnectDeny,		// server -> client, server full or shutting down
		ETP_Disconnect,			// either direction, a clean goodbye
		ETP_KeepAlive,			// either direction, keeps a connection from timing out
		ETP_Payload,			// carries a game message
		ETP_Max
	};

	// Delivery guarantee requested for a payload.
	enum ETransportChannel : uint8_t
	{
		ETC_Unreliable = 0,	// fire and forget -- input, anything superseded next tick
		ETC_Reliable,		// retransmitted until acked, delivered in order
	};

#pragma pack(push, 1)
	// Packed so the layout is exactly what goes on the wire, with no padding.
	// Multi-byte fields are little-endian; both ends are x86-64 for now, and this
	// is the place to add byte-order conversion if that ever stops being true.
	struct TransportHeader
	{
		uint32_t protocolId = TRANSPORT_PROTOCOL_ID;
		uint8_t type = ETP_Max;

		// Assigned by the server on accept. Zero during the handshake, and on any
		// packet from a client that is not yet connected.
		uint32_t connectionId = 0;

		// --- reliability ---
		// Every packet carries an ack for the peer, even handshake and keep-alive
		// traffic, so acks keep flowing while no payloads are being sent.
		uint16_t sequence = 0;	// this packet's sequence number
		uint16_t ack = 0;		// highest sequence seen from the peer
		uint32_t ackBits = 0;	// the 32 sequences before `ack`, bit 0 = ack-1

		uint8_t channel = ETC_Unreliable;

		// Ordering position within the reliable stream. Only meaningful when
		// channel is ETC_Reliable; a retransmit reuses it, so the receiver can
		// order and de-duplicate independently of the packet sequence.
		uint16_t messageId = 0;

		// --- fragmentation ---
		// A message too large for one datagram is split across several. Each
		// fragment is its own reliable message with its own consecutive messageId,
		// so the ordering machinery already guarantees they arrive in order and
		// none are missing -- reassembly is then just an accumulate.
		uint8_t fragmentCount = 1;	// 1 means the message is not fragmented
		uint8_t fragmentIndex = 0;
	};
#pragma pack(pop)

	static_assert(sizeof(TransportHeader) == 22, "TransportHeader must have no padding");

	// Connection tuning. Milliseconds.
	constexpr uint64_t CONNECT_REQUEST_INTERVAL_MS = 250;	// resend while connecting
	constexpr uint64_t CONNECT_TIMEOUT_MS = 5000;			// give up connecting
	constexpr uint64_t KEEP_ALIVE_INTERVAL_MS = 1000;		// idle heartbeat
	constexpr uint64_t CONNECTION_TIMEOUT_MS = 5000;		// drop a silent peer

	constexpr uint32_t MAX_TRANSPORT_PACKET_BYTES = 1200;	// conservative, under typical MTU
	constexpr uint32_t MAX_CONNECTIONS = 8;

	// Reliability tuning.
	constexpr uint64_t RELIABLE_RESEND_INTERVAL_MS = 100;	// resend an unacked packet after this
	constexpr uint32_t MAX_UNACKED_RELIABLE = 256;			// send window before we stop accepting more

	// The most datagrams one tick may retransmit, per connection.
	//
	// Without a cap, a full send window resends all 256 at once, every interval --
	// 2560 datagrams a second against a peer that drains 64 a frame. The socket
	// buffer overflows, the acks that would have retired those packets are among
	// what is lost, and the window refills faster than it drains. The cap is what
	// keeps a backlog recoverable instead of self-sustaining, and it stays under
	// what a single ack can confirm.
	constexpr uint32_t MAX_RESENDS_PER_TICK = 24;
	constexpr uint32_t MAX_ORDER_BUFFER = 256;				// out-of-order messages held while waiting

	// The largest payload a single packet can carry.
	constexpr uint32_t MAX_PAYLOAD_BYTES = MAX_TRANSPORT_PACKET_BYTES - sizeof(TransportHeader);

	// fragmentCount is a byte, so this is the ceiling on a single message. Roughly
	// 300 KB, comfortably above a full world state; anything larger is refused
	// outright rather than silently truncated.
	constexpr uint32_t MAX_FRAGMENTS = 255;
	constexpr uint32_t MAX_MESSAGE_BYTES = MAX_FRAGMENTS * MAX_PAYLOAD_BYTES;

	// True when `a` is more recent than `b`, accounting for 16-bit wraparound.
	inline bool SequenceGreaterThan(const uint16_t a, const uint16_t b)
	{
		constexpr uint16_t half = 32768;
		return ((a > b) && (a - b <= half)) || ((a < b) && (b - a > half));
	}
}
