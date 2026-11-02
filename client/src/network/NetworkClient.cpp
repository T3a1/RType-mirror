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

constexpr std::chrono::milliseconds AckDelay{50};

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
        if (m_channel.needsAck() && now - m_lastSent >= AckDelay) {
            send(Ack{}, now);
        }
    }
    return received;
}

void NetworkClient::sendInput(std::uint8_t actions, TimePoint now)
{
    if (m_state != State::Connected) {
        return;
    }
    send(Input{.sequence = ++m_inputSequence, .actions = actions}, now);
}

void NetworkClient::receivePending(TimePoint now,
                                   std::vector<Payload> &received)
{
    while (const auto datagram =
               m_socket->receive(m_buffer, std::chrono::milliseconds::zero())) {
        if (datagram->sender != m_server) {
            continue;
        }
        const auto result =
            decodePacket({m_buffer.data(), datagram->size}, Side::Server);
        if (const auto *decodeError = std::get_if<DecodeError>(&result)) {
            if (*decodeError == DecodeError::BadVersion) {
                BB_ERROR("The server runs another protocol version");
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
        BB_ERROR("Rejected by the server ("
                 << (reject->reason == RejectReason::ServerFull
                         ? "server full"
                         : "version mismatch")
                 << ")");
        m_state = State::Rejected;
    } else if (std::holds_alternative<Disconnect>(packet.payload)) {
        BB_INFO("Disconnected by the server");
        m_state = State::Closed;
    } else if (!std::holds_alternative<Ack>(packet.payload)) {
        received.push_back(packet.payload);
    }
}

void NetworkClient::send(const Payload &payload, TimePoint now)
{
    Packet packet{.payload = payload};
    if (!m_channel.prepare(packet, now)) {
        return;
    }
    ByteWriter writer;
    if (!encodePacket(packet, writer)) {
        return;
    }
    const std::error_code error =
        m_socket->sendTo(m_server, {writer.data().data(), writer.size()});
    if (error) {
        BB_DEBUG("Send failed: " << error.message());
    }
    m_lastSent = now;
}

} // namespace client
