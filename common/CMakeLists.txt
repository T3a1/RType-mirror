cmake_minimum_required(VERSION 3.15)
project(r-type_common)

set(SOURCE_FILES
    src/systems/MovementSystem.cpp
    src/systems/PlayerControlSystem.cpp
    src/protocol/ByteWriter.cpp
    src/protocol/ByteReader.cpp
    src/protocol/PacketHeader.cpp
    src/protocol/Messages.cpp
    src/protocol/Packet.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME}
    PUBLIC include
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC r-type_engine
)
