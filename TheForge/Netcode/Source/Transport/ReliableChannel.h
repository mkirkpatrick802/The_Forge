#pragma once
#include <cstdint>
#include <map>
#include <vector>

#include "TransportProtocol.h"

namespace NetCode
{
	// A delivered payload, handed up to the game once ordering is satisfied.
	using TransportMessage = std::vector<uint8_t>;

	// A reliable payload waiting for its turn in the ordered stream. The header
	// travels with it because fragment reassembly needs it at delivery time, not
	// at arrival time.
	struct BufferedMessage
	{
		TransportHeader header;
		TransportMessage data;
	};

	// Per-connection reliability: sequencing, acks, retransmission and ordered
	// delivery of the reliable stream.
	//
	// This owns no socket. UdpTransport asks it what bytes to put on the wire and
	// feeds it what came off, which keeps the retransmission logic testable and
	// free of platform detail.
	class ReliableChannel
	{
	public:
		// Fills in sequence, ack and ackBits for an outgoing packet, and consumes a
		// sequence number. Used for handshake and keep-alive packets, which carry
		// acks even though they have no payload.
		void StampOutgoing(TransportHeader& header);

		// Builds the datagram(s) for a payload and, when reliable, retains them for
		// retransmission. A reliable message larger than one datagram is split into
		// consecutive fragments, so this can produce several. Returns false if the
		// message is too large to fragment or the send window is full.
		bool QueuePayload(ETransportChannel channel, const void* data, uint32_t size, uint32_t connectionId, std::vector<std::vector<uint8_t>>& outDatagrams);

		// Records the packet for ack generation and applies the peer's acks.
		// Returns false if the packet should be ignored (a duplicate).
		bool ProcessIncomingHeader(const TransportHeader& header);

		// Feeds a reliable payload into the ordering buffer, appending anything that
		// has become deliverable. Unreliable payloads bypass this entirely.
		void ProcessIncomingPayload(const TransportHeader& header, const uint8_t* payload, uint32_t size, std::vector<TransportMessage>& outDelivered);

		// Datagrams whose ack has not arrived within the resend interval.
		//
		// A retransmit goes out under a *new* sequence number, and is re-keyed here
		// under it. Sequence identifies a datagram, not a message: the receiver only
		// acknowledges sequences within 32 of the newest it has seen, so resending
		// under the original one means a packet that ever falls behind that window
		// can never be acknowledged again -- it would sit in the send window forever,
		// be resent forever, and eventually fill it. Ordering and de-duplication are
		// unaffected because they key off messageId, which is deliberately preserved.
		void CollectResends(uint64_t nowMs, std::vector<const std::vector<uint8_t>*>& outDatagrams);

		void MarkSent(uint16_t sequence, uint64_t nowMs);

		// True when a payload has been received that we have not yet acknowledged.
		// Acks only travel on outgoing packets, so a receiver that has nothing to
		// say would otherwise not ack until its next keep-alive -- once a second,
		// against a sender pushing 30 updates a second. The sender's window fills,
		// it retransmits everything it is holding, and the resulting storm is what
		// actually loses packets. Deliberately set for payloads only: if plain
		// acks set it too, both ends would ack each other's acks forever.
		bool HasUnsentAcks() const { return _hasUnsentAcks; }

		// True once enough packets have arrived unacknowledged that waiting for the
		// end of the frame would put some of them out of reach.
		//
		// An ack covers the newest sequence plus the 32 before it, so a peer can only
		// ever confirm 33 packets at a time. The send window holds up to
		// MAX_UNACKED_RELIABLE, far more than that -- so a burst bigger than the ack
		// window (a full world state, or a backlog being retransmitted) leaves the
		// oldest packets permanently unacknowledgeable, resent forever, filling the
		// sender's window for good. Acking mid-burst is what keeps what is in flight
		// inside what an ack can describe.
		bool NeedsImmediateAck() const { return _unackedReceiveCount >= ACK_BURST_THRESHOLD; }

		uint32_t GetUnackedCount() const { return static_cast<uint32_t>(_unacked.size()); }
		uint32_t GetResentCount() const { return _resentCount; }

	private:
		// Appends a single fully-ordered message, reassembling fragments as they
		// arrive. Only called once ordering has already been satisfied.
		void DeliverOrdered(const TransportHeader& header, const uint8_t* payload, uint32_t size, std::vector<TransportMessage>& outDelivered);

		void ApplyAcks(uint16_t ack, uint32_t ackBits);
		void RecordReceived(uint16_t sequence);
		void WriteAckFields(std::vector<uint8_t>& datagram) const;

		// Stamps a fresh sequence and current acks onto an already-built datagram,
		// and returns the sequence used. For retransmits.
		uint16_t RestampDatagram(std::vector<uint8_t>& datagram);

	private:
		struct UnackedPacket
		{
			std::vector<uint8_t> datagram;
			uint64_t lastSentMs = 0;
		};

		// Outgoing
		uint16_t _localSequence = 0;
		uint16_t _nextMessageId = 0;
		std::map<uint16_t, UnackedPacket> _unacked;
		uint32_t _resentCount = 0;

		// Incoming ack generation
		uint16_t _remoteSequence = 0;
		uint32_t _receivedBits = 0;
		bool _hasReceived = false;
		bool _hasUnsentAcks = false;

		// Comfortably inside the 33 an ack can carry, leaving room for reordering.
		static constexpr uint32_t ACK_BURST_THRESHOLD = 24;
		uint32_t _unackedReceiveCount = 0;

		// Incoming ordering
		uint16_t _nextExpectedMessageId = 0;
		std::map<uint16_t, BufferedMessage> _orderBuffer;

		// Fragment reassembly. Ordering guarantees fragments arrive in order and
		// contiguously, so this only ever needs to accumulate the current message.
		std::vector<uint8_t> _reassembly;
		uint8_t _expectedFragmentIndex = 0;
		uint8_t _expectedFragmentCount = 0;
	};
}
