cmake_minimum_required(VERSION 3.15)
project(r-type_server)

set(SOURCE_FILES
    src/main.cpp
    src/Game.cpp
    src/systems/EnemyAISystem.cpp
    src/systems/EventBroadcastSystem.cpp
    src/systems/NetworkInputSystem.cpp
    src/systems/SessionSystem.cpp
    src/systems/SnapshotSystem.cpp
    src/systems/BoundarySystem.cpp
    src/systems/LifetimeSystem.cpp
)

# The end of the game is a library so the tests can run it without the game loop.
add_library(r-type_server_game_over STATIC
    src/systems/GameOverSystem.cpp
    src/systems/RoomPhaseSystem.cpp
)

target_include_directories(r-type_server_game_over
    PUBLIC include
)

target_link_libraries(r-type_server_game_over
    PUBLIC r-type_common
)

# The network thread is a library so the tests can run it on a fake socket.
add_library(r-type_server_network STATIC
    src/network/NetworkServer.cpp
    src/network/AdminCommands.cpp
    src/AccessList.cpp
)

target_include_directories(r-type_server_network
    PUBLIC include
)

target_link_libraries(r-type_server_network
    PUBLIC r-type_common
)

# Level files: Conan's nlohmann_json, or the Alpine package nlohmann-json in
# the Dockerfile (header-only).
find_package(nlohmann_json 3.11 CONFIG QUIET)
if(NOT nlohmann_json_FOUND)
    find_path(NLOHMANN_JSON_INCLUDE_DIR nlohmann/json.hpp REQUIRED)
    add_library(nlohmann_json::nlohmann_json INTERFACE IMPORTED)
    target_include_directories(nlohmann_json::nlohmann_json
        INTERFACE ${NLOHMANN_JSON_INCLUDE_DIR}
    )
endif()

# Spawning, levels and combat (collisions, damage, deaths) are a library so
# the tests can run them without the game loop.
add_library(r-type_server_spawn STATIC
    src/EnemyCatalog.cpp
    src/EnemyFile.cpp
    src/EntityFactory.cpp
    src/Formations.cpp
    src/Level.cpp
    src/Despawn.cpp
    src/Weapons.cpp
    src/systems/BossSystem.cpp
    src/systems/CollisionSystem.cpp
    src/systems/DamageSystem.cpp
    src/systems/DeathSystem.cpp
    src/systems/LevelSystem.cpp
    src/systems/ObstacleSystem.cpp
    src/systems/PowerUpSystem.cpp
    src/systems/ShootSystem.cpp
    src/systems/ShotStyleSystems.cpp
    src/systems/SpawnCommandSystem.cpp
    src/systems/SpawnSystem.cpp
)

target_include_directories(r-type_server_spawn
    PUBLIC include
)

target_link_libraries(r-type_server_spawn
    PUBLIC r-type_common
    PRIVATE nlohmann_json::nlohmann_json
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME}
    PRIVATE include
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE r-type_server_network r-type_server_spawn
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE r-type_server_game_over
)

# The levels, the weapons and the enemies are read next to the executable.
# A target of their own copies them at every build, so that a change to a
# data file reaches the build directory even when no code was rebuilt. It
# names the executable's directory itself: $<TARGET_FILE_DIR> would make it
# depend on the server, which depends on it.
get_property(MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(MULTI_CONFIG)
    set(SERVER_DATA_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>)
else()
    set(SERVER_DATA_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()
add_custom_target(r-type_server_data ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/levels
        ${SERVER_DATA_DIR}/levels
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/weapons.json
        ${SERVER_DATA_DIR}/weapons.json
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/enemies.json
        ${SERVER_DATA_DIR}/enemies.json
    COMMENT "Copying the server levels, weapons and enemies"
)
add_dependencies(${PROJECT_NAME} r-type_server_data)
