/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** GameOverSystem
*/

#include "systems/GameOverSystem.hpp"
#include "common/components/Health.hpp"
#include "common/components/Player.hpp"
#include "common/protocol/Messages.hpp"

namespace server {

void gameOverSystem(engine::Registry &registry, Players &players,
                    Outbound &outbound, bool &gameOver)
{
    bool anyAlive = false;
    for (auto &[clientId, player] : players) {
        if (const auto *ship = registry.tryGet<rtype::Player>(player.ship)) {
            player.score = ship->score;
        }
        const auto *health = registry.tryGet<rtype::Health>(player.ship);
        anyAlive = anyAlive || (health != nullptr && health->current > 0);
    }
    if (anyAlive || players.empty()) {
        gameOver = false;
        return;
    }
    if (!gameOver) {
        gameOver = true;
        outbound.push(OutboundMessage{
            .to = Recipient::All,
            .payload =
                rtype::protocol::GameEvent{
                    .kind = rtype::protocol::GameEventKind::GameOver,
                },
        });
    }
}

} // namespace server
