#include "ReliableChannel.h"

#include <cstddef>
#include <cstring>
#include <ranges>

#include "Engine/System.h"

namespace
{
	// Byte offsets of the ack fields inside TransportHeader, so a retransmitted
	// datagram can have them refreshed without rebuilding it.
	constexpr size_t SEQUENCE_OFFSET = offsetof(NetCode::TransportHeader, sequence);
	constexpr size_t ACK_OFFSET = offsetof(NetCode::TransportHeader, ack);
	constexpr size_t ACK_BITS_OFFSET = offsetof(NetCode::TransportHeader, ackBits);
}

void NetCode::ReliableChannel::StampOutgoing(TransportHeader& header)
{
	header.sequence = _localSequence++;
	header.ack = _remoteSequence;
	header.ackBits = _receivedBits;

	// Whatever this packet is, it carries our acks with it.
	_hasUnsentAcks = false;
	_unackedReceiveCount = 0;
}

void NetCode::ReliableChannel::WriteAckFields(std::vector<uint8_t>& datagram) const
{
	if (datagram.size() < sizeof(TransportHeader)) return;

	std::memcpy(datagram.data() + ACK_OFFSET, &_remoteSequence, sizeof(_remoteSequence));
	std::memcpy(datagram.data() + ACK_BITS_OFFSET, &_receivedBits, sizeof(_receivedBits));
}

bool NetCode::ReliableChannel::QueuePayload(const ETransportChannel channel, const void* data, const uint32_t size, const uint32_t connectionId, std::vector<std::vector<uint8_t>>& outDatagrams)
{
	// Only the reliable channel fragments. The large things (world state) need
	// reliability anyway, and unreliable traffic (input) is small by nature --
	// reassembling a message whose pieces may never arrive is not worth the
	// machinery.
	if (channel != ETC_Reliable && size > MAX_PAYLOAD_BYTES)
	{
		DEBUG_LOG("Transport: unreliable payload of %u bytes exceeds the %u byte limit; send it reliably to fragment.", size, MAX_PAYLOAD_BYTES)
		return false;
	}

	if (size > MAX_MESSAGE_BYTES)
	{
		DEBUG_LOG("Transport: message of %u bytes exceeds the %u byte maximum.", size, MAX_MESSAGE_BYTES)
		return false;
	}

	const uint32_t fragmentCount = size <= MAX_PAYLOAD_BYTES
		? 1u
		: (size + MAX_PAYLOAD_BYTES - 1) / MAX_PAYLOAD_BYTES;

	if (channel == ETC_Reliable && _unacked.size() + fragmentCount > MAX_UNACKED_RELIABLE)
	{
		// Names the connection: on a server with several clients, "the window is full"
		// without saying whose cannot distinguish one stalled peer from a general
		// backlog, and those want opposite responses.
		DEBUG_LOG("Transport: reliable send window full for connection %u (%u unacked, needed %u more).", connectionId, GetUnackedCount(), fragmentCount)
		return false;
	}

	const auto* bytes = static_cast<const uint8_t*>(data);

	for (uint32_t index = 0; index < fragmentCount; ++index)
	{
		const uint32_t offset = index * MAX_PAYLOAD_BYTES;
		const uint32_t chunk = (size - offset) < MAX_PAYLOAD_BYTES ? (size - offset) : MAX_PAYLOAD_BYTES;

		TransportHeader header;
		header.type = ETP_Payload;
		header.connectionId = connectionId;
		header.channel = channel;
		header.fragmentCount = static_cast<uint8_t>(fragmentCount);
		header.fragmentIndex = static_cast<uint8_t>(index);
		StampOutgoing(header);

		// Each fragment is its own reliable message, so ordering delivers them in
		// sequence and none can go missing.
		if (channel == ETC_Reliable)
			header.messageId = _nextMessageId++;

		std::vector<uint8_t> datagram(sizeof(header) + chunk);
		std::memcpy(datagram.data(), &header, sizeof(header));
		if (chunk > 0)
			std::memcpy(datagram.data() + sizeof(header), bytes + offset, chunk);

		if (channel == ETC_Reliable)
		{
			UnackedPacket pending;
			pending.datagram = datagram;
			pending.lastSentMs = 0; // set by MarkSent once it actually goes out
			_unacked.emplace(header.sequence, std::move(pending));
		}

		outDatagrams.push_back(std::move(datagram));
	}

	return true;
}

void NetCode::ReliableChannel::MarkSent(const uint16_t sequence, const uint64_t nowMs)
{
	if (const auto it = _unacked.find(sequence); it != _unacked.end())
		it->second.lastSentMs = nowMs;
}

bool NetCode::ReliableChannel::ProcessIncomingHeader(const TransportHeader& header)
{
	RecordReceived(header.sequence);
	ApplyAcks(header.ack, header.ackBits);
	return true;
}

void NetCode::ReliableChannel::RecordReceived(const uint16_t sequence)
{
	++_unackedReceiveCount;

	if (!_hasReceived)
	{
		_hasReceived = true;
		_remoteSequence = sequence;
		_receivedBits = 0;
		return;
	}

	if (SequenceGreaterThan(sequence, _remoteSequence))
	{
		// Newer than anything seen: shift the history along by the gap and record
		// the old newest in the bitfield.
		const uint16_t shift = sequence - _remoteSequence;
		_receivedBits = shift >= 32 ? 0u : ((_receivedBits << shift) | (1u << (shift - 1)));
		_remoteSequence = sequence;
		return;
	}

	// Older: set its bit if it still falls inside the 32-packet history.
	const uint16_t age = _remoteSequence - sequence;
	if (age > 0 && age <= 32)
		_receivedBits |= (1u << (age - 1));
}

void NetCode::ReliableChannel::ApplyAcks(const uint16_t ack, const uint32_t ackBits)
{
	_unacked.erase(ack);

	for (uint32_t bit = 0; bit < 32; ++bit)
	{
		if ((ackBits & (1u << bit)) == 0) continue;

		const uint16_t sequence = static_cast<uint16_t>(ack - (bit + 1));
		_unacked.erase(sequence);
	}
}

uint16_t NetCode::ReliableChannel::RestampDatagram(std::vector<uint8_t>& datagram)
{
	if (datagram.size() < sizeof(TransportHeader)) return 0;

	const uint16_t sequence = _localSequence++;
	std::memcpy(datagram.data() + SEQUENCE_OFFSET, &sequence, sizeof(sequence));

	// Refresh the acks too, so a retransmit is as informative as a fresh packet.
	WriteAckFields(datagram);
	_hasUnsentAcks = false;

	return sequence;
}

void NetCode::ReliableChannel::CollectResends(const uint64_t nowMs, std::vector<const std::vector<uint8_t>*>& outDatagrams)
{
	// Pulled out first, because each one is about to be re-keyed under a new
	// sequence and the map cannot be rearranged while it is being walked.
	std::vector<UnackedPacket> due;
	for (auto it = _unacked.begin(); it != _unacked.end();)
	{
		if (due.size() >= MAX_RESENDS_PER_TICK) break;

		UnackedPacket& pending = it->second;
		if (pending.lastSentMs == 0 || nowMs - pending.lastSentMs < RELIABLE_RESEND_INTERVAL_MS)
		{
			++it;
			continue;
		}

		due.push_back(std::move(pending));
		it = _unacked.erase(it);
	}

	for (auto& pending : due)
	{
		const uint16_t sequence = RestampDatagram(pending.datagram);
		pending.lastSentMs = nowMs;
		++_resentCount;

		// std::map keeps its values put, so handing out a pointer into it is safe.
		const auto [entry, inserted] = _unacked.emplace(sequence, std::move(pending));
		outDatagrams.push_back(&entry->second.datagram);
	}
}

void NetCode::ReliableChannel::ProcessIncomingPayload(const TransportHeader& header, const uint8_t* payload, const uint32_t size, std::vector<TransportMessage>& outDelivered)
{
	_hasUnsentAcks = true;

	if (header.channel != ETC_Reliable)
	{
		// Unreliable payloads are delivered immediately, in whatever order they
		// arrive, and are never retained or reassembled.
		outDelivered.emplace_back(payload, payload + size);
		return;
	}

	// Already delivered: a retransmit of something we have moved past.
	if (SequenceGreaterThan(_nextExpectedMessageId, header.messageId))
		return;

	if (header.messageId == _nextExpectedMessageId)
	{
		DeliverOrdered(header, payload, size, outDelivered);
		++_nextExpectedMessageId;

		// Anything buffered behind this one can now be delivered too.
		for (auto it = _orderBuffer.find(_nextExpectedMessageId); it != _orderBuffer.end(); it = _orderBuffer.find(_nextExpectedMessageId))
		{
			DeliverOrdered(it->second.header, it->second.data.data(), static_cast<uint32_t>(it->second.data.size()), outDelivered);
			_orderBuffer.erase(it);
			++_nextExpectedMessageId;
		}

		return;
	}

	// Arrived early: hold it until the gap is filled. emplace() leaves an existing
	// entry alone, which de-duplicates retransmits of a buffered message.
	if (_orderBuffer.size() >= MAX_ORDER_BUFFER)
	{
		DEBUG_LOG("Transport: order buffer full, dropping message %u.", header.messageId)
		return;
	}

	_orderBuffer.emplace(header.messageId, BufferedMessage{ header, TransportMessage(payload, payload + size) });
}

void NetCode::ReliableChannel::DeliverOrdered(const TransportHeader& header, const uint8_t* payload, const uint32_t size, std::vector<TransportMessage>& outDelivered)
{
	// The common case: a whole message in one datagram.
	if (header.fragmentCount <= 1)
	{
		outDelivered.emplace_back(payload, payload + size);
		return;
	}

	if (header.fragmentIndex == 0)
	{
		_reassembly.clear();
		_expectedFragmentIndex = 0;
		_expectedFragmentCount = header.fragmentCount;
	}
	else if (header.fragmentIndex != _expectedFragmentIndex || header.fragmentCount != _expectedFragmentCount)
	{
		// Ordering should make this impossible; if it ever happens the message is
		// unrecoverable, so drop the partial rather than emit something corrupt.
		DEBUG_LOG("Transport: fragment %u/%u out of step, discarding partial message.", header.fragmentIndex, header.fragmentCount)
		_reassembly.clear();
		_expectedFragmentCount = 0;
		return;
	}

	_reassembly.insert(_reassembly.end(), payload, payload + size);
	++_expectedFragmentIndex;

	if (_expectedFragmentIndex == _expectedFragmentCount)
	{
		outDelivered.push_back(std::move(_reassembly));
		_reassembly.clear();
		_expectedFragmentCount = 0;
		_expectedFragmentIndex = 0;
	}
}
