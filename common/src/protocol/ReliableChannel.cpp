/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** ReliableChannel
*/

#include "common/protocol/ReliableChannel.hpp"

namespace rtype::protocol {

bool ReliableChannel::prepare(Packet &packet, TimePoint now)
{
    const PacketType type = payloadType(packet.payload);
    const bool reliable = isReliable(type);
    if (reliable && m_pending.size() >= MaxPendingReliable) {
        return false;
    }
    packet.header.type = type;
    packet.header.flags = reliable ? PacketFlag::Reliable : 0;
    packet.header.sequence = reliable ? m_nextReliable++ : m_nextUnreliable++;
    packet.header.ack = m_lastInOrder;
    m_needsAck = false;
    if (reliable) {
        m_pending.push_back(
            {.packet = packet, .resendAt = now + ResendInterval});
    }
    return true;
}

bool ReliableChannel::receive(const PacketHeader &header)
{
    const auto lastSent = static_cast<std::uint16_t>(m_nextReliable - 1);
    if (!isNewer(header.ack, lastSent)) {
        while (!m_pending.empty() &&
               !isNewer(m_pending.front().packet.header.sequence, header.ack)) {
            m_pending.pop_front();
        }
    }
    if (!isReliable(header.type)) {
        return true;
    }
    m_needsAck = true;
    if (header.sequence != static_cast<std::uint16_t>(m_lastInOrder + 1)) {
        return false;
    }
    m_lastInOrder = header.sequence;
    return true;
}

std::vector<Packet> ReliableChannel::resendsDue(TimePoint now)
{
    std::vector<Packet> due;
    for (Pending &pending : m_pending) {
        if (pending.resendAt <= now) {
            pending.packet.header.ack = m_lastInOrder;
            pending.resendAt = now + ResendInterval;
            due.push_back(pending.packet);
        }
    }
    if (!due.empty()) {
        m_needsAck = false;
    }
    return due;
}

} // namespace rtype::protocol
