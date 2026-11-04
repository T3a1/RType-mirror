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
    src/systems/BoundarySystem.cpp
    src/systems/ShootSystem.cpp
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

# Spawning and levels are a library so the tests can run them without the
# game loop.
add_library(r-type_server_spawn STATIC
    src/EntityFactory.cpp
    src/Formations.cpp
    src/Level.cpp
    src/systems/BossSystem.cpp
    src/systems/LevelSystem.cpp
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

# The levels are read next to the executable.
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/levels
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/levels
    COMMENT "Copying the server levels"
)
