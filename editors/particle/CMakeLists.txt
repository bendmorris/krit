cmake_minimum_required(VERSION 3.9 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Building Debug by default as no build type was specified (-DCMAKE_BUILD_TYPE=)")
    set(CMAKE_BUILD_TYPE "Debug")
else()
    message(STATUS "Building ${CMAKE_BUILD_TYPE}")
endif()

# engine

set(KRIT_DIR "${CMAKE_SOURCE_DIR}/../..")
set(KRIT_INCLUDE_PATHS ${CMAKE_SOURCE_DIR}/src)
set(KRIT_ENABLE_TOOLS ON)
include(${KRIT_DIR}/CMakeLists.txt)

# editor

project(ped)

file(GLOB_RECURSE SRC_FILES
    CONFIGURE_DEPENDS
    src/*.cpp
    src/**/*.cpp
)

include_directories(src)

add_executable(${PROJECT_NAME} ${SRC_FILES})
target_link_libraries(${PROJECT_NAME}
    krit
)
