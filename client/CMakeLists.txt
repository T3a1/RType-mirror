cmake_minimum_required(VERSION 3.15)
project(r-type_client)

# Graphics backend: the client only uses client/include/graphics/IGraphics.hpp,
# each backend implements it in src/graphics/<backend>/ and links its library.
set(RTYPE_GRAPHICS_BACKEND "raylib" CACHE STRING "Graphics library of the client")
set_property(CACHE RTYPE_GRAPHICS_BACKEND PROPERTY STRINGS raylib sfml ncurses)

set(SOURCE_FILES
    src/main.cpp
    src/Game.cpp
    src/RoomPages.cpp
    src/SpriteSheets.cpp
    src/systems/AnimationSystem.cpp
    src/systems/AudioSystem.cpp
    src/Bench.cpp
    src/systems/BackgroundRenderSystem.cpp
    src/systems/BackgroundScrollSystem.cpp
    src/systems/HudSystem.cpp
    src/systems/InputSystem.cpp
    src/systems/NetworkSyncSystem.cpp
    src/systems/RenderSystem.cpp
)

if(RTYPE_GRAPHICS_BACKEND STREQUAL "raylib")
    find_package(raylib REQUIRED)
    list(APPEND SOURCE_FILES src/graphics/raylib/RaylibGraphics.cpp)
    list(APPEND SOURCE_FILES src/audio/raylib/RaylibAudio.cpp)
    set(GRAPHICS_LIBRARIES raylib)
elseif(RTYPE_GRAPHICS_BACKEND STREQUAL "sfml")
    find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System Audio)
    list(APPEND SOURCE_FILES src/graphics/sfml/SfmlGraphics.cpp)
    list(APPEND SOURCE_FILES src/graphics/sfml/ExecutableDirectory.cpp)
    list(APPEND SOURCE_FILES src/audio/sfml/SfmlAudio.cpp)
    set(GRAPHICS_LIBRARIES SFML::Graphics SFML::Audio)
elseif(RTYPE_GRAPHICS_BACKEND STREQUAL "ncurses")
    if(WIN32)
        message(FATAL_ERROR "The ncurses client only runs on Linux")
    endif()
    find_package(Curses REQUIRED)
    list(APPEND SOURCE_FILES src/graphics/ncurses/NcursesGraphics.cpp)
    list(APPEND SOURCE_FILES src/audio/NullAudio.cpp)
    set(GRAPHICS_LIBRARIES Curses::Curses)
else()
    message(FATAL_ERROR "Unknown RTYPE_GRAPHICS_BACKEND: ${RTYPE_GRAPHICS_BACKEND}")
endif()
message(STATUS "Client graphics backend: ${RTYPE_GRAPHICS_BACKEND}")

# The connection to the server is a library so the tests can run it on a
# fake socket.
add_library(r-type_client_network STATIC src/network/NetworkClient.cpp)

target_include_directories(r-type_client_network
    PUBLIC include
)

target_link_libraries(r-type_client_network
    PUBLIC r-type_common
)

# The key bindings are a library so the tests can read controls files.
add_library(r-type_client_controls STATIC src/Controls.cpp)

target_include_directories(r-type_client_controls
    PUBLIC include
)

# The sounds of the game are a library so the tests can run them on a fake
# backend.
add_library(r-type_client_audio STATIC src/audio/SoundBoard.cpp)

target_include_directories(r-type_client_audio
    PUBLIC include
)

# The settings are a library so the tests can read and write settings files.
add_library(r-type_client_settings STATIC src/Settings.cpp)

target_include_directories(r-type_client_settings
    PUBLIC include
)

# The chat logic is a library so the tests can run it without a window.
add_library(r-type_client_chat STATIC src/Chat.cpp)

target_include_directories(r-type_client_chat
    PUBLIC include
)

target_link_libraries(r-type_client_chat
    PUBLIC r-type_common
)

# The menu widgets are a library so the tests can run them on a fake backend.
add_library(r-type_client_menu STATIC src/KeyCapture.cpp src/Menu.cpp
    src/MenuPages.cpp)

target_include_directories(r-type_client_menu
    PUBLIC include
)

target_link_libraries(r-type_client_menu
    PUBLIC r-type_engine r-type_client_audio r-type_client_controls
    r-type_client_settings
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME}
    PRIVATE include
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE r-type_client_network r-type_client_controls r-type_client_audio
    r-type_client_menu r-type_client_settings r-type_client_chat
    ${GRAPHICS_LIBRARIES}
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
    COMMENT "Copying the client assets"
)
