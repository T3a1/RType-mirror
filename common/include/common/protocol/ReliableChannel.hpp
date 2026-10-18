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

/**
 * @class ReliableChannel
 * @brief The reliability state of one connection: sequence numbering,
 * acknowledgments and resends (PROTOCOL.md, section 7).
 *
 * Each side keeps one channel per peer. Call prepare() before sending any
 * packet, receive() for every valid packet received, and resendsDue()
 * regularly. It never touches a socket or a clock: the time is passed in.
 **/
class ReliableChannel {
  public:
    using TimePoint = std::chrono::steady_clock::time_point;

    /**
     * @brief Fills the header of an outgoing packet (all but payloadSize)
     * and keeps a copy of reliable packets until they are acknowledged.
     * @param packet The packet to send; its header is overwritten.
     * @param now The current time, to schedule the first resend.
     * @return `false` if MaxPendingReliable reliable packets are already
     * waiting: the peer must be disconnected.
     **/
    [[nodiscard]] bool prepare(Packet &packet, TimePoint now);

    /**
     * @brief Applies the ack of a received packet, which forgets the
     * acknowledged copies, then checks the order of reliable packets.
     * @param header The header of a packet that decoded correctly.
     * @return `false` if the packet must be dropped: a reliable duplicate,
     * or a reliable packet that arrived before a lost one.
     **/
    [[nodiscard]] bool receive(const PacketHeader &header);

    /**
     * @brief Returns the reliable packets not acknowledged for ResendInterval,
     * with an up-to-date ack, and schedules their next resend.
     * @param now The current time.
     * @return The packets to send again, already prepared.
     **/
    [[nodiscard]] std::vector<Packet> resendsDue(TimePoint now);

    /**
     * @brief Tells whether a reliable packet was received since the last
     * packet sent, so an Ack must be sent if nothing else goes out.
     * @return `true` if an acknowledgment is owed to the peer.
     **/
    [[nodiscard]] bool needsAck() const { return m_needsAck; }

    /**
     * @brief Returns the number of reliable packets waiting for an ack.
     * @return The number of stored copies.
     **/
    [[nodiscard]] std::size_t pendingCount() const { return m_pending.size(); }

    /**
     * @brief Returns the last reliable sequence received in order, the value
     * sent in the ack field.
     * @return The sequence, or 0 if nothing was received yet.
     **/
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
