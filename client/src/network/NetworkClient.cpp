/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** NetworkClient
*/

#include "network/NetworkClient.hpp"
#include "common/protocol/ByteWriter.hpp"
#include "common/protocol/Connection.hpp"
#include "common/protocol/ReliableChannel.hpp"
#include "engine/Logger.hpp"

#include <asio.hpp>

#include <array>

namespace client {

namespace {

using namespace rtype::protocol;

constexpr std::chrono::milliseconds AckDelay{50};

} // namespace

struct NetworkClient::Impl {
    asio::io_context io;
    asio::ip::udp::socket socket{io};
    asio::ip::udp::endpoint server;
    ReliableChannel channel;
    std::array<std::uint8_t, MaxPacketSize> buffer{};
};

NetworkClient::NetworkClient(const std::string &host, std::uint16_t port)
    : m_impl(std::make_unique<Impl>())
{
    asio::ip::udp::resolver resolver(m_impl->io);
    m_impl->server =
        *resolver.resolve(asio::ip::udp::v4(), host, std::to_string(port))
             .begin();
    m_impl->socket.open(asio::ip::udp::v4());
    m_impl->socket.non_blocking(true);
    BB_INFO("Connecting to " << m_impl->server.address() << ":"
                             << m_impl->server.port());
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
        if (m_impl->channel.needsAck() && now - m_lastSent >= AckDelay) {
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
    asio::ip::udp::endpoint sender;
    asio::error_code error;
    while (true) {
        const std::size_t size = m_impl->socket.receive_from(
            asio::buffer(m_impl->buffer), sender, 0, error);
        if (error == asio::error::would_block) {
            return;
        }
        if (error || sender != m_impl->server) {
            continue;
        }
        const auto result =
            decodePacket({m_impl->buffer.data(), size}, Side::Server);
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
        if (!m_impl->channel.receive(packet.header)) {
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
    if (!m_impl->channel.prepare(packet, now)) {
        return;
    }
    ByteWriter writer;
    if (!encodePacket(packet, writer)) {
        return;
    }
    asio::error_code error;
    m_impl->socket.send_to(asio::buffer(writer.data().data(), writer.size()),
                           m_impl->server, 0, error);
    if (error) {
        BB_DEBUG("Send failed: " << error.message());
    }
    m_lastSent = now;
}

} // namespace client
