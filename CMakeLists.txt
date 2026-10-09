cmake_minimum_required(VERSION 3.10)
project(r-type)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

option(BUILD_CLIENT "Build the client (requires the Conan packages)" ON)
option(BUILD_TESTS "Build the unit tests (requires GoogleTest)" OFF)
option(ENABLE_COVERAGE "Instrument the code for test coverage (GCC/Clang)" OFF)

if(ENABLE_COVERAGE)
    add_compile_options(--coverage -O0 -g)
    add_link_options(--coverage)
endif()

add_subdirectory(engine)
add_subdirectory(common)
add_subdirectory(server)

if(BUILD_CLIENT)
    add_subdirectory(client)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
