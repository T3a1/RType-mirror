cmake_minimum_required(VERSION 3.15)
project(r-type_tests)

find_package(GTest REQUIRED)
include(GoogleTest)

set(SOURCE_FILES
    src/sanity_test.cpp
    src/sparse_set_test.cpp
    src/registry_test.cpp
    src/vec2_test.cpp
    src/movement_system_test.cpp
    src/pattern_system_test.cpp
    src/formations_test.cpp
    src/player_control_system_test.cpp
    src/interpolation_system_test.cpp
    src/prediction_system_test.cpp
    src/message_queue_test.cpp
    src/logger_test.cpp
    src/byte_io_test.cpp
    src/packet_test.cpp
    src/packet_fuzz_test.cpp
    src/reliable_channel_test.cpp
    src/udp_socket_test.cpp
    src/network_server_test.cpp
    src/game_over_system_test.cpp
)

# The client's network code only exists when the client is built.
if(TARGET r-type_client_network)
    list(APPEND SOURCE_FILES src/network_client_test.cpp)
    set(CLIENT_NETWORK_LIBRARY r-type_client_network)
endif()

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME}
    PRIVATE r-type_server_network r-type_server_spawn ${CLIENT_NETWORK_LIBRARY}
    GTest::gtest GTest::gtest_main GTest::gmock
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE r-type_server_game_over
)

gtest_discover_tests(${PROJECT_NAME})
