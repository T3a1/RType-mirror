cmake_minimum_required(VERSION 3.10)
project(r-type_common)

set(SOURCE_FILES
    src/systems/MovementSystem.cpp
    src/systems/PlayerControlSystem.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME}
    PUBLIC include
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC r-type_engine
)
