cmake_minimum_required(VERSION 3.15)
project(r-type_common)

# Network backend: the programs only use engine/include/engine/network/ and
# include/common/network/UdpSocket.hpp, each backend implements them in
# src/network/<backend>/ and links its library.
set(RTYPE_NETWORK_BACKEND "asio" CACHE STRING "Network library of the programs")
set_property(CACHE RTYPE_NETWORK_BACKEND PROPERTY STRINGS asio)

set(SOURCE_FILES
    src/systems/MovementSystem.cpp
    src/systems/PatternSystem.cpp
    src/systems/PlayerControlSystem.cpp
    src/systems/InterpolationSystem.cpp
    src/systems/PredictionSystem.cpp
    src/protocol/ByteWriter.cpp
    src/protocol/ByteReader.cpp
    src/protocol/PacketHeader.cpp
    src/protocol/Messages.cpp
    src/protocol/Packet.cpp
    src/protocol/ReliableChannel.cpp
)

if(RTYPE_NETWORK_BACKEND STREQUAL "asio")
    # Conan's asio package, or the Alpine package asio-dev in the Dockerfile.
    find_package(asio CONFIG QUIET)
    if(NOT asio_FOUND)
        find_path(ASIO_INCLUDE_DIR asio.hpp REQUIRED)
        add_library(asio::asio INTERFACE IMPORTED)
        target_include_directories(asio::asio INTERFACE ${ASIO_INCLUDE_DIR})
        target_compile_definitions(asio::asio INTERFACE ASIO_STANDALONE)
    endif()
    list(APPEND SOURCE_FILES src/network/asio/AsioUdpSocket.cpp)
    set(NETWORK_LIBRARIES asio::asio)
else()
    message(FATAL_ERROR "Unknown RTYPE_NETWORK_BACKEND: ${RTYPE_NETWORK_BACKEND}")
endif()
message(STATUS "Network backend: ${RTYPE_NETWORK_BACKEND}")

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME}
    PUBLIC include
)

find_package(Threads REQUIRED)

# The network library stays PRIVATE: no public header includes it, so the
# server, the client and the tests never see it.
target_link_libraries(${PROJECT_NAME}
    PUBLIC r-type_engine Threads::Threads
    PRIVATE ${NETWORK_LIBRARIES}
)
