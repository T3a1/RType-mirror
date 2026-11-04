/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** FakeRooms
*/

#ifndef TESTS_FAKEROOMS_HPP
#define TESTS_FAKEROOMS_HPP

#include "engine/IInstance.hpp"
#include "network/NetworkMessages.hpp"

#include <chrono>
#include <memory>

namespace tests {

/**
 * @brief A game that only forwards the messages of its room between the queues
 * of the room and the queues of a test.
 **/
class ForwardingGame final : public engine::IInstance {
  public:
    ForwardingGame(server::Rooms::Link &link, server::Inbound &inbound,
                   server::Outbound &outbound)
        : m_link(link), m_inbound(inbound), m_outbound(outbound)
    {
    }

    void tick(engine::Duration /*dt*/) override
    {
        forwardInbound();
        for (const auto &message : m_outbound.drain()) {
            m_link.outbound.push(message);
        }
    }

    void onStop() override { forwardInbound(); }

  private:
    void forwardInbound()
    {
        for (const auto &message : m_link.inbound.drain()) {
            m_inbound.push(message);
        }
    }

    server::Rooms::Link &m_link;
    server::Inbound &m_inbound;
    server::Outbound &m_outbound;
};

/**
 * @struct FakeRooms
 * @brief A single room of MaxPlayers players, whose messages a test reads and
 * writes directly in inbound and outbound.
 * @param inbound The messages the room received.
 * @param outbound The messages for the room to send.
 * @param rooms The rooms to give to a NetworkServer.
 **/
struct FakeRooms {
    server::Inbound inbound;
    server::Outbound outbound;
    server::Rooms rooms{
        [this](server::Rooms::Link &link) {
            return std::make_unique<ForwardingGame>(link, inbound, outbound);
        },
        {.maxInstances = 1, .maxMembers = rtype::protocol::MaxPlayers},
        std::chrono::milliseconds(1),
    };
};

} // namespace tests

#endif // TESTS_FAKEROOMS_HPP
