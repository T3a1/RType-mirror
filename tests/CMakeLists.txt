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
    src/player_control_system_test.cpp
    src/message_queue_test.cpp
    src/logger_test.cpp
    src/byte_io_test.cpp
    src/packet_test.cpp
    src/packet_fuzz_test.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME}
    PRIVATE r-type_common GTest::gtest GTest::gtest_main GTest::gmock
)

gtest_discover_tests(${PROJECT_NAME})
