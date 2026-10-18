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

/**
 * @class NetworkClient
 * @brief The client side of the connection to the server, without a thread.
 *
 * The game calls update() then sendInput() once per frame. It handles the
 * protocol itself (Connect retries, Accept, Reject, Disconnect, acks,
 * timeout) and only hands the game what concerns it. No Asio type appears
 * in this header, so it can be included next to raylib.h.
 **/
class NetworkClient {
  public:
    using TimePoint = std::chrono::steady_clock::time_point;

    /**
     * @enum State
     * @brief Where the connection stands. Every state but Connecting and
     * Connected is final.
     * @param Connecting Sending Connect every ConnectRetryInterval.
     * @param Connected Accepted by the server.
     * @param Rejected Refused: server full or another protocol version.
     * @param Lost Nothing received from the server for PeerTimeout.
     * @param Closed The server ended the connection.
     **/
    enum class State : std::uint8_t {
        Connecting,
        Connected,
        Rejected,
        Lost,
        Closed,
    };

    /**
     * @brief Resolves the server address and opens a non-blocking UDP
     * socket. The first Connect is sent by the first update().
     * @param host The server's host name or IP address.
     * @param port The server's UDP port.
     * @throws asio::system_error If the host cannot be resolved or the
     * socket cannot be opened.
     **/
    NetworkClient(const std::string &host, std::uint16_t port);

    /**
     * @brief Sends Disconnect{Left} if connected, so the server frees the
     * slot at once instead of waiting for the timeout.
     **/
    ~NetworkClient();

    NetworkClient(const NetworkClient &) = delete;
    NetworkClient &operator=(const NetworkClient &) = delete;
    NetworkClient(NetworkClient &&) = delete;
    NetworkClient &operator=(NetworkClient &&) = delete;

    /**
     * @brief Reads every pending datagram from the server, then sends what
     * is due: Connect while connecting, or an Ack owed for 50 ms. Detects
     * the timeout. Does nothing in a final state.
     * @param now The current time.
     * @return The payloads the game needs (Welcome, Snapshot, GameEvent), in
     * the order they arrived.
     **/
    [[nodiscard]] std::vector<rtype::protocol::Payload> update(TimePoint now);

    /**
     * @brief Sends the actions held this frame, with a new sequence number.
     * Must be called every frame, even with no key held: it keeps the
     * connection alive and carries the ack. Does nothing until connected.
     * @param actions The held actions, as Action bits.
     * @param now The current time.
     **/
    void sendInput(std::uint8_t actions, TimePoint now);

    /**
     * @brief Returns where the connection stands, for the status display.
     * @return The current state.
     **/
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
