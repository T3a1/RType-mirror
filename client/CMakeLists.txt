cmake_minimum_required(VERSION 3.10)
project(r-type_client)

find_package(raylib REQUIRED)

set(SOURCE_FILES
    src/main.cpp
    src/Game.cpp
    src/systems/RenderSystem.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME}
    PRIVATE include
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE r-type_common raylib
)
