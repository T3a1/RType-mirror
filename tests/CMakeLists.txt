cmake_minimum_required(VERSION 3.10)
project(r-type_tests)

find_package(GTest REQUIRED)
include(GoogleTest)

set(SOURCE_FILES
    src/sanity_test.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME}
    PRIVATE GTest::gtest GTest::gtest_main GTest::gmock
)

gtest_discover_tests(${PROJECT_NAME})
