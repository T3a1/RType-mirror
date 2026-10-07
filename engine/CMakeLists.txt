cmake_minimum_required(VERSION 3.10)
project(r-type_engine)

set(SOURCE_FILES
    src/Clock.cpp
    src/Registry.cpp
    src/Scheduler.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME}
    PUBLIC include
)
