/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** game_over_system_test
*/

#include "Players.hpp"
#include "common/components/Health.hpp"
#include "common/components/Player.hpp"
#include "common/protocol/Messages.hpp"
#include "engine/Entity.hpp"
#include "engine/Registry.hpp"
#include "network/NetworkMessages.hpp"
#include "systems/GameOverSystem.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <variant>

namespace {

class GameOverSystem : public testing::Test {
  public:
    engine::Entity addPlayer(std::uint32_t clientId, std::uint32_t score = 0)
    {
        const engine::Entity ship = registry.create();
        registry.emplace<rtype::Player>(
            ship, rtype::Player{.slot = 1, .score = score});
        registry.emplace<rtype::Health>(ship,
                                        rtype::Health{.current = 1, .max = 1});
        players[clientId] = server::ConnectedPlayer{.ship = ship, .slot = 1};
        return ship;
    }

    void kill(engine::Entity ship)
    {
        registry.get<rtype::Health>(ship).current = 0;
    }

    std::size_t runAndCountGameOvers()
    {
        server::gameOverSystem(registry, players, outbound, gameOver);
        std::size_t count = 0;
        for (const server::OutboundMessage &message : outbound.drain()) {
            const auto *event =
                std::get_if<rtype::protocol::GameEvent>(&message.payload);
            if (event != nullptr &&
                event->kind == rtype::protocol::GameEventKind::GameOver) {
                ++count;
            }
        }
        return count;
    }

    engine::Registry registry;
    server::Players players;
    server::Outbound outbound;
    bool gameOver = false;
};

} // namespace

TEST_F(GameOverSystem, NothingWithoutPlayers)
{
    EXPECT_EQ(runAndCountGameOvers(), 0U);
    EXPECT_FALSE(gameOver);
}

TEST_F(GameOverSystem, NothingWhileAPlayerIsAlive)
{
    kill(addPlayer(1));
    addPlayer(2);

    EXPECT_EQ(runAndCountGameOvers(), 0U);
}

TEST_F(GameOverSystem, SentOnceWhenEveryShipIsDead)
{
    kill(addPlayer(1));
    kill(addPlayer(2));

    EXPECT_EQ(runAndCountGameOvers(), 1U);
    EXPECT_EQ(runAndCountGameOvers(), 0U);
}

TEST_F(GameOverSystem, DestroyedShipCountsAsDead)
{
    registry.destroy(addPlayer(1));
    registry.flush();

    EXPECT_EQ(runAndCountGameOvers(), 1U);
}

TEST_F(GameOverSystem, ScoreSurvivesTheShip)
{
    const engine::Entity ship = addPlayer(1, 700);
    kill(ship);
    EXPECT_EQ(runAndCountGameOvers(), 1U);

    registry.destroy(ship);
    registry.flush();
    EXPECT_EQ(runAndCountGameOvers(), 0U);

    EXPECT_EQ(players.at(1).score, 700U);
}

TEST_F(GameOverSystem, LastAlivePlayerLeavingEndsTheGame)
{
    kill(addPlayer(1));
    addPlayer(2);
    EXPECT_EQ(runAndCountGameOvers(), 0U);

    players.erase(2);

    EXPECT_EQ(runAndCountGameOvers(), 1U);
}

TEST_F(GameOverSystem, NewPlayerStartsANewGame)
{
    kill(addPlayer(1));
    EXPECT_EQ(runAndCountGameOvers(), 1U);

    const engine::Entity ship = addPlayer(2);
    EXPECT_EQ(runAndCountGameOvers(), 0U);
    EXPECT_FALSE(gameOver);

    kill(ship);
    EXPECT_EQ(runAndCountGameOvers(), 1U);
}
