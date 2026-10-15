cmake_minimum_required(VERSION 3.15)
project(r-type_client)

find_package(raylib REQUIRED)

set(SOURCE_FILES
    src/main.cpp
    src/Game.cpp
    src/EnemyFactory.cpp
    src/systems/BackgroundRenderSystem.cpp
    src/systems/BackgroundScrollSystem.cpp
    src/systems/DebugSpawnSystem.cpp
    src/systems/InputSystem.cpp
    src/systems/RenderSystem.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME}
    PRIVATE include
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE r-type_common raylib
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
    COMMENT "Copying the client assets"
)
