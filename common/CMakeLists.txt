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
    src/protocol/ReliableChannel.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME}
    PUBLIC include
)

find_package(Threads REQUIRED)
find_package(asio CONFIG QUIET)
if(NOT asio_FOUND)
    find_path(ASIO_INCLUDE_DIR asio.hpp REQUIRED)
    add_library(asio::asio INTERFACE IMPORTED)
    target_include_directories(asio::asio INTERFACE ${ASIO_INCLUDE_DIR})
    target_compile_definitions(asio::asio INTERFACE ASIO_STANDALONE)
endif()

target_link_libraries(${PROJECT_NAME}
    PUBLIC r-type_engine asio::asio Threads::Threads
)
