/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** NetworkClient
*/

#ifndef CLIENT_NETWORK_NETWORKCLIENT_HPP
#define CLIENT_NETWORK_NETWORKCLIENT_HPP

#include "common/protocol/Packet.hpp"

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace client {

class NetworkClient {
  public:
    using TimePoint = std::chrono::steady_clock::time_point;

    enum class State : std::uint8_t {
        Connecting,
        Connected,
        Rejected,
        Lost,
        Closed,
    };

    NetworkClient(const std::string &host, std::uint16_t port);
    ~NetworkClient();

    NetworkClient(const NetworkClient &) = delete;
    NetworkClient &operator=(const NetworkClient &) = delete;
    NetworkClient(NetworkClient &&) = delete;
    NetworkClient &operator=(NetworkClient &&) = delete;

    [[nodiscard]] std::vector<rtype::protocol::Payload> update(TimePoint now);
    void sendInput(std::uint8_t actions, TimePoint now);

    [[nodiscard]] State state() const { return m_state; }

  private:
    struct Impl;

    void receivePending(TimePoint now,
                        std::vector<rtype::protocol::Payload> &received);
    void handlePacket(const rtype::protocol::Packet &packet,
                      std::vector<rtype::protocol::Payload> &received);
    void send(const rtype::protocol::Payload &payload, TimePoint now);

    std::unique_ptr<Impl> m_impl;
    State m_state = State::Connecting;
    TimePoint m_lastHeard;
    TimePoint m_lastSent;
    TimePoint m_nextConnect;
    std::uint32_t m_inputSequence = 0;
};

} // namespace client

#endif // CLIENT_NETWORK_NETWORKCLIENT_HPP
