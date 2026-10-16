/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** EntityFactory
*/

#ifndef SERVER_ENTITYFACTORY_HPP
#define SERVER_ENTITYFACTORY_HPP

#include "EnemyDefs.hpp"
#include "SpawnRequests.hpp"
#include "common/components/Collider.hpp"
#include "common/components/Damage.hpp"
#include "common/components/Enemy.hpp"
#include "common/components/EntityType.hpp"
#include "common/components/Health.hpp"
#include "common/components/InputState.hpp"
#include "common/components/Lifetime.hpp"
#include "common/components/NetworkId.hpp"
#include "common/components/Player.hpp"
#include "common/components/Projectile.hpp"
#include "common/components/Transform.hpp"
#include "common/components/Velocity.hpp"
#include "common/components/Weapon.hpp"
#include "components/DestroyOffScreen.hpp"
#include "components/KeepInPlayfield.hpp"
#include "engine/Entity.hpp"
#include "engine/Logger.hpp"
#include "engine/Registry.hpp"

#include <cstdint>

namespace server {

/**
 * Builds every gameplay entity of the server.
 * Each create() assembles a complete entity from a spawn request
 * and gives it a unique NetworkId.
 **/
class EntityFactory {
  public:
    engine::Entity create(engine::Registry &registry,
                          const SpawnEnemy &request);
    engine::Entity create(engine::Registry &registry,
                          const SpawnPlayer &request);
    engine::Entity create(engine::Registry &registry,
                          const SpawnProjectile &request);

  private:
    std::uint32_t m_nextNetworkId = 1;
};

} // namespace server

#endif // SERVER_ENTITYFACTORY_HPP
