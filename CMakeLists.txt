cmake_minimum_required(VERSION 3.10)
project(r-type)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

option(BUILD_CLIENT "Build the client (requires the Conan packages)" ON)

add_subdirectory(server)

if(BUILD_CLIENT)
    add_subdirectory(client)
endif()
