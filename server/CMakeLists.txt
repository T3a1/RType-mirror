cmake_minimum_required(VERSION 3.15)
project(r-type_server)

set(SOURCE_FILES
    src/main.cpp
    src/Game.cpp
    src/Despawn.cpp
    src/systems/CollisionSystem.cpp
    src/systems/DamageSystem.cpp
    src/systems/DeathSystem.cpp
    src/systems/EnemyAISystem.cpp
    src/systems/EventBroadcastSystem.cpp
    src/systems/NetworkInputSystem.cpp
    src/systems/SessionSystem.cpp
    src/systems/SnapshotSystem.cpp
    src/systems/SpawnSystem.cpp
    src/systems/BoundarySystem.cpp
    src/systems/ShootSystem.cpp
    src/systems/LifetimeSystem.cpp
    src/EntityFactory.cpp
)

# The network thread is a library so the tests can run it on a fake socket.
add_library(r-type_server_network STATIC src/network/NetworkServer.cpp)

target_include_directories(r-type_server_network
    PUBLIC include
)

target_link_libraries(r-type_server_network
    PUBLIC r-type_common
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME}
    PRIVATE include
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE r-type_server_network
)
