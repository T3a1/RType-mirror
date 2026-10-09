cmake_minimum_required(VERSION 3.10)
project(r-type_common)

# Header-only for now: becomes a STATIC library when the protocol adds sources.
add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME}
    INTERFACE include
)

target_link_libraries(${PROJECT_NAME}
    INTERFACE r-type_engine
)
