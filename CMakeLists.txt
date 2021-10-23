project(krit)
cmake_policy (SET CMP0072 OLD)

option(BUILD_WINDOWS "Cross-compile for Windows using mingw-w64" OFF)
option(ENABLE_TOOLS "Cross-compile for Windows using mingw-w64" OFF)
option(ENABLE_THREADS "Whether task managers should use multiple threads" ON)
option(USE_GLEW "Whether to use glew instead of direct GL" ON)

if(BUILD_WINDOWS)
    include(${KRIT_DIR}/toolchain/windows.cmake)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    include(${KRIT_DIR}/toolchain/emscripten.cmake)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    include(${KRIT_DIR}/toolchain/linux.cmake)
else()
    message(FATAL_ERROR "Unrecognized platform")
endif()

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -O3 -ffast-math -ffunction-sections -fdata-sections -fno-exceptions")
set(CMAKE_CXX_FLAGS_PROF "${CMAKE_CXX_FLAGS_PROF} ${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -ffast-math -fno-exceptions -g")
set(CMAKE_CXX_FLAGS_ASAN "${CMAKE_CXX_FLAGS_ASAN} ${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fsanitize=undefined")
set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -lmcheck")
set(CMAKE_LINKER_FLAGS_ASAN "${CMAKE_LINKER_FLAGS_ASAN} ${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined")

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
find_package(OpenGL REQUIRED)
endif()

if(ENABLE_TOOLS)
    message(STATUS "tools enabled")
    add_definitions(-DKRIT_ENABLE_TOOLS)
    file(GLOB_RECURSE TOOLS_FILES CONFIGURE_DEPENDS ${KRIT_DIR}/imgui/*.cpp)
    include_directories(${KRIT_DIR}/imgui)
endif()

file(GLOB_RECURSE KRIT_SRC_FILES CONFIGURE_DEPENDS ${KRIT_DIR}/src/krit/*.cpp)
file(GLOB_RECURSE KRIT_THIRDPARTY_FILES CONFIGURE_DEPENDS ${KRIT_DIR}/spine-runtimes/spine-cpp/spine-cpp/src/spine/*.cpp)

include_directories(${KRIT_INCLUDE_PATHS})
include_directories(${KRIT_DIR}/src)
include_directories(${KRIT_DIR}/spine-runtimes/spine-cpp/spine-cpp/include)
include_directories(${KRIT_DIR}/quickjs)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../)

# string(STRIP ${SDL2_LIBRARIES} SDL2_LIBRARIES)

file(GLOB_RECURSE KRIT_GENERATED_SCRIPT_FILES
    CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/krit/script/ScriptClass.*.cpp
)

add_library(${PROJECT_NAME} STATIC
    ${CMAKE_CURRENT_BINARY_DIR}/krit/Assets.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/krit/script/ScriptBridge.cpp
    ${KRIT_GENERATED_SCRIPT_FILES}
    ${KRIT_SRC_FILES}
    ${KRIT_THIRDPARTY_FILES}
    ${TOOLS_FILES}
)

if(ENABLE_THREADS)
    add_definitions(-DKRIT_ENABLE_THREADS)
endif()
if(USE_GLEW)
    add_definitions(-DKRIT_USE_GLEW)
endif()

include_directories(krit/src)
include_directories(krit/spine-runtimes/spine-cpp/spine-cpp/include)
include_directories(krit/imgui)
include_directories(krit/quickjs)
include_directories(/usr/include/SDL2)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

message(STATUS "toolchain library path:${TOOLCHAIN_LIB_PATH} prefixes:${CMAKE_FIND_LIBRARY_PREFIXES} suffixes:${CMAKE_FIND_LIBRARY_SUFFIXES}")

find_library(EXPAT_LIB NAMES expat HINTS "${TOOLCHAIN_LIB_PATH}" CMAKE_FIND_ROOT_PATH_BOTH)
find_library(FREETYPE_LIB NAMES freetype freetype-6 HINTS "${TOOLCHAIN_LIB_PATH}" CMAKE_FIND_ROOT_PATH_BOTH)
find_library(HARFBUZZ_LIB NAMES harfbuzz HINTS "${TOOLCHAIN_LIB_PATH}" CMAKE_FIND_ROOT_PATH_BOTH)
find_library(QUICKJS_LIB NAMES quickjs HINTS "${TOOLCHAIN_LIB_PATH}" CMAKE_FIND_ROOT_PATH_BOTH)
find_library(SDL_LIB NAMES SDL2 HINTS "${TOOLCHAIN_LIB_PATH}" CMAKE_FIND_ROOT_PATH_BOTH)
find_library(SNDFILE_LIB NAMES sndfile HINTS "${TOOLCHAIN_LIB_PATH}" CMAKE_FIND_ROOT_PATH_BOTH)
find_library(YAML_LIB NAMES yaml HINTS "${TOOLCHAIN_LIB_PATH}" CMAKE_FIND_ROOT_PATH_BOTH)
find_library(Z_LIB NAMES z HINTS "${TOOLCHAIN_LIB_PATH}" CMAKE_FIND_ROOT_PATH_BOTH)
find_library(ZIP_LIB NAMES zip HINTS "${TOOLCHAIN_LIB_PATH}" CMAKE_FIND_ROOT_PATH_BOTH)
if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    set(OPENAL_LIB "-lopenal")
    set(GLEW_LIB "")
    set(OPENGL_LIBRARIES "-lGLEW -lGLU -lm -lGL")
else()
    find_library(GLEW_LIB NAMES GLEW glew32 HINTS "${TOOLCHAIN_LIB_PATH}" CMAKE_FIND_ROOT_PATH_BOTH)
    find_library(OPENAL_LIB NAMES openal HINTS "${TOOLCHAIN_LIB_PATH}" CMAKE_FIND_ROOT_PATH_BOTH)
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(SDL_IMAGE_LIB "-lSDL2_image")
else()
    find_library(SDL_IMAGE_LIB NAMES SDL2_image HINTS "${TOOLCHAIN_LIB_PATH}" CMAKE_FIND_ROOT_PATH_BOTH)
endif()
if(ENABLE_THREADS)
    set(PTHREAD_LIB "-lpthread")
else()
    set(PTHREAD_LIB "")
endif()

target_link_libraries(${PROJECT_NAME}
    PUBLIC
    ${OPENGL_LIBRARIES}
    ${OPENAL_LIB}
    ${SDL_LIB}
    ${SDL_IMAGE_LIB}
    ${HARFBUZZ_LIB}
    ${GLEW_LIB}
    ${FREETYPE_LIB}

    ${STATIC}
    ${ZIP_LIB}
    ${Z_LIB}
    ${SNDFILE_LIB}
    ${QUICKJS_LIB}
    ${EXPAT_LIB}
    ${YAML_LIB}
    ${PTHREAD_LIB}
    "-Wl,--gc-sections"
    "-static-libgcc"
)
target_include_directories(${PROJECT_NAME} PRIVATE ${FREETYPE_INCLUDE_DIRS})

add_custom_target(
    compile-assets ALL
    COMMAND cd ${CMAKE_BINARY_DIR} && python3 ${KRIT_DIR}/tools/asset_registry/asset_registry.py --input ${CMAKE_SOURCE_DIR}/assets.yaml --output-dir ${CMAKE_CURRENT_BINARY_DIR}/krit
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/krit/Assets.h ${CMAKE_CURRENT_BINARY_DIR}/krit/Assets.cpp
)
add_dependencies(${PROJECT_NAME} compile-assets)

add_custom_target(
    copy-assets ALL
    COMMAND
        ${KRIT_DIR}/tools/update_assets.sh ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}
    BYPRODUCTS assets.zip
)
add_dependencies(compile-assets copy-assets)

if(KRIT_SCRIPT_BRIDGE_PATH)
    add_custom_target(
        generate-bridge ALL
        mkdir -p "${CMAKE_CURRENT_BINARY_DIR}/krit"
        COMMAND cd ${KRIT_DIR}/tools/bridge_generator && node src/index.js ${KRIT_SCRIPT_BRIDGE_PATH} "${CMAKE_CURRENT_BINARY_DIR}/krit"
        BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/krit/script/ScriptClass.h ${CMAKE_CURRENT_BINARY_DIR}/krit/script/ScriptClass.cpp ${CMAKE_CURRENT_BINARY_DIR}/krit/script/ScriptBridge.h ${CMAKE_CURRENT_BINARY_DIR}/krit/script/ScriptBridge.cpp
    )
    add_dependencies(${PROJECT_NAME} generate-bridge)
endif()
