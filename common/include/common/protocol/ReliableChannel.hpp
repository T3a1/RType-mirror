/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** ReliableChannel
*/

#ifndef COMMON_PROTOCOL_RELIABLECHANNEL_HPP
#define COMMON_PROTOCOL_RELIABLECHANNEL_HPP

#include "common/protocol/Connection.hpp"
#include "common/protocol/Packet.hpp"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <vector>

namespace rtype::protocol {

class ReliableChannel {
  public:
    using TimePoint = std::chrono::steady_clock::time_point;

    // Fills the header but payloadSize. False if the peer has too many
    // unacknowledged reliable packets: it must be disconnected.
    [[nodiscard]] bool prepare(Packet &packet, TimePoint now);

    // False if the packet must be dropped (duplicate or out of order).
    [[nodiscard]] bool receive(const PacketHeader &header);

    [[nodiscard]] std::vector<Packet> resendsDue(TimePoint now);

    [[nodiscard]] bool needsAck() const { return m_needsAck; }
    [[nodiscard]] std::size_t pendingCount() const { return m_pending.size(); }
    [[nodiscard]] std::uint16_t lastInOrder() const { return m_lastInOrder; }

  private:
    struct Pending {
        Packet packet;
        TimePoint resendAt;
    };

    std::deque<Pending> m_pending;
    std::uint16_t m_nextReliable = FirstReliableSequence;
    std::uint16_t m_nextUnreliable = 0;
    std::uint16_t m_lastInOrder = 0;
    bool m_needsAck = false;
};

} // namespace rtype::protocol

#endif // COMMON_PROTOCOL_RELIABLECHANNEL_HPP
