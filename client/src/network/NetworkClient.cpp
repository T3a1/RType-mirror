/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** NetworkClient
*/

#include "network/NetworkClient.hpp"
#include "common/network/UdpSocket.hpp"
#include "common/protocol/ByteWriter.hpp"
#include "common/protocol/Connection.hpp"
#include "engine/Logger.hpp"

#include <utility>

namespace client {

namespace {

using namespace rtype::protocol;

/** @brief How long an ack may wait for another packet to carry it. */
constexpr std::chrono::milliseconds AckDelay{50};

/**
 * @brief How long the same actions are not sent again: about 30 Input packets
 *per second at any frame rate, so that a fast client does not fill a slow link.
 **/
constexpr std::chrono::milliseconds InputInterval{30};

} // namespace

NetworkClient::NetworkClient(const std::string &host, std::uint16_t port)
    : NetworkClient(rtype::network::openUdpSocket(),
                    rtype::network::resolve(host, port))
{
}

NetworkClient::NetworkClient(std::unique_ptr<engine::net::IUdpSocket> socket,
                             const engine::net::Endpoint &server)
    : m_socket(std::move(socket)), m_server(server)
{
    BB_INFO("Connecting to " << m_server.toString());
}

NetworkClient::~NetworkClient()
{
    if (m_state == State::Connected) {
        send(Disconnect{.reason = DisconnectReason::Left},
             std::chrono::steady_clock::now());
    }
}

std::vector<Payload> NetworkClient::update(TimePoint now)
{
    std::vector<Payload> received;
    if (m_state != State::Connecting && m_state != State::Connected) {
        return received;
    }
    receivePending(now, received);
    if (m_state == State::Connecting && now >= m_nextConnect) {
        send(Connect{}, now);
        m_nextConnect = now + ConnectRetryInterval;
    }
    if (m_state == State::Connected) {
        if (now - m_lastHeard > PeerTimeout) {
            BB_WARN("Server stopped answering");
            m_state = State::Lost;
            return received;
        }
        for (const Packet &packet : m_channel.resendsDue(now)) {
            sendPacket(packet, now);
        }
        if (m_channel.needsAck() && now - m_lastSent >= AckDelay) {
            send(Ack{}, now);
        }
    }
    return received;
}

std::uint32_t NetworkClient::sendInput(std::uint8_t actions, TimePoint now)
{
    if (m_state != State::Connected) {
        return 0;
    }
    if (actions == m_lastActions && now < m_nextInput) {
        return m_inputSequence + 1;
    }
    send(Input{.sequence = ++m_inputSequence, .actions = actions}, now);
    m_lastActions = actions;
    m_nextInput = now + InputInterval;
    return m_inputSequence;
}

void NetworkClient::joinRoom(std::uint32_t room, TimePoint now)
{
    if (m_state == State::Connected) {
        send(JoinRoom{.room = room}, now);
    }
}

void NetworkClient::leaveRoom(TimePoint now)
{
    if (m_state == State::Connected) {
        send(LeaveRoom{}, now);
    }
}

void NetworkClient::startGame(const StartGame &settings, TimePoint now)
{
    if (m_state == State::Connected) {
        send(settings, now);
    }
}

void NetworkClient::sendChat(const std::string &text, TimePoint now)
{
    if (m_state == State::Connected) {
        send(rtype::protocol::ChatSend{.text = text}, now);
    }
}

void NetworkClient::receivePending(TimePoint now,
                                   std::vector<Payload> &received)
{
    while (const auto datagram =
               m_socket->receive(m_buffer, std::chrono::milliseconds::zero())) {
        if (datagram->sender != m_server) {
            continue;
        }
        m_bytesReceived += datagram->size;
        const auto result =
            decodePacket({m_buffer.data(), datagram->size}, Side::Server);
        if (const auto *decodeError = std::get_if<DecodeError>(&result)) {
            if (*decodeError == DecodeError::BadVersion) {
                BB_ERROR("The server runs another protocol version");
                m_rejectReason = RejectReason::VersionMismatch;
                m_state = State::Rejected;
                return;
            }
            continue;
        }
        const auto &packet = std::get<Packet>(result);
        m_lastHeard = now;
        if (!m_channel.receive(packet.header)) {
            continue;
        }
        handlePacket(packet, received);
        if (m_state != State::Connecting && m_state != State::Connected) {
            return;
        }
    }
}

void NetworkClient::handlePacket(const Packet &packet,
                                 std::vector<Payload> &received)
{
    if (std::holds_alternative<Accept>(packet.payload)) {
        BB_INFO("Connected to the server");
        m_state = State::Connected;
    } else if (const auto *reject = std::get_if<Reject>(&packet.payload)) {
        BB_ERROR("Rejected by the server, reason "
                 << static_cast<int>(reject->reason));
        m_rejectReason = reject->reason;
        m_state = State::Rejected;
    } else if (const auto *closed = std::get_if<Disconnect>(&packet.payload)) {
        BB_INFO("Disconnected by the server");
        m_disconnectReason = closed->reason;
        m_state = State::Closed;
    } else if (!std::holds_alternative<Ack>(packet.payload)) {
        received.push_back(packet.payload);
    }
}

void NetworkClient::send(const Payload &payload, TimePoint now)
{
    Packet packet{.payload = payload};
    if (m_channel.prepare(packet, now)) {
        sendPacket(packet, now);
    }
}

void NetworkClient::sendPacket(const Packet &packet, TimePoint now)
{
    ByteWriter writer;
    if (!encodePacket(packet, writer)) {
        return;
    }
    const std::error_code error =
        m_socket->sendTo(m_server, {writer.data().data(), writer.size()});
    if (error) {
        BB_DEBUG("Send failed: " << error.message());
    } else {
        m_bytesSent += writer.size();
    }
    m_lastSent = now;
}

} // namespace client
