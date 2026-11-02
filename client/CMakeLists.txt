cmake_minimum_required(VERSION 3.15)
project(r-type_client)

# Graphics backend: the client only uses client/include/graphics/IGraphics.hpp,
# each backend implements it in src/graphics/<backend>/ and links its library.
set(RTYPE_GRAPHICS_BACKEND "raylib" CACHE STRING "Graphics library of the client")
set_property(CACHE RTYPE_GRAPHICS_BACKEND PROPERTY STRINGS raylib sfml)

set(SOURCE_FILES
    src/main.cpp
    src/Game.cpp
    src/systems/BackgroundRenderSystem.cpp
    src/systems/BackgroundScrollSystem.cpp
    src/network/NetworkClient.cpp
    src/systems/HudSystem.cpp
    src/systems/InputSystem.cpp
    src/systems/NetworkSyncSystem.cpp
    src/systems/RenderSystem.cpp
)

if(RTYPE_GRAPHICS_BACKEND STREQUAL "raylib")
    find_package(raylib REQUIRED)
    list(APPEND SOURCE_FILES src/graphics/raylib/RaylibGraphics.cpp)
    set(GRAPHICS_LIBRARIES raylib)
elseif(RTYPE_GRAPHICS_BACKEND STREQUAL "sfml")
    find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System)
    list(APPEND SOURCE_FILES src/graphics/sfml/SfmlGraphics.cpp)
    set(GRAPHICS_LIBRARIES SFML::Graphics)
else()
    message(FATAL_ERROR "Unknown RTYPE_GRAPHICS_BACKEND: ${RTYPE_GRAPHICS_BACKEND}")
endif()
message(STATUS "Client graphics backend: ${RTYPE_GRAPHICS_BACKEND}")

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME}
    PRIVATE include
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE r-type_common ${GRAPHICS_LIBRARIES}
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
    COMMENT "Copying the client assets"
)
