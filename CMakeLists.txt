project(krit)
cmake_policy(SET CMP0072 NEW)
cmake_policy(SET CMP0077 NEW)

option(BUILD_WINDOWS "Cross-compile for Windows using mingw-w64" OFF)
option(KRIT_ENABLE_TOOLS "Include imgui and devtools functionality" OFF)
option(KRIT_ENABLE_THREADS "Whether task managers should use multiple threads" ON)
option(KRIT_ENABLE_MULTISAMPLING "Whether to use multisampling for framebuffers" ON)
option(KRIT_USE_GLEW "Whether to use glew instead of direct GL" ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -MMD -MF .obj/.d -Wno-array-bounds -Wno-format-truncation -D_GNU_SOURCE -DCONFIG_VERSION=\\\"2021-03-27\\\" -fno-omit-frame-pointer -fasynchronous-unwind-tables")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE} -O2")
set(CMAKE_C_FLAGS_PROF "${CMAKE_C_FLAGS_RELEASE} ${CMAKE_C_FLAGS_PROF} -DTRACY_ENABLE")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-parentheses -Wno-unused-result -Wall -ffunction-sections -fdata-sections -Os -fno-rtti")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -ffunction-sections -fdata-sections -DKRIT_RELEASE")
set(CMAKE_CXX_FLAGS_PROF "${CMAKE_CXX_FLAGS_RELEASE} ${CMAKE_CXX_FLAGS_PROF} -DTRACY_ENABLE -Wno-sometimes-uninitialized")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_ASAN "${CMAKE_CXX_FLAGS_DEBUG} ${CMAKE_CXX_FLAGS_ASAN} -fsanitize=address -fsanitize=undefined")

set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -rdynamic")
set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -lmcheck")
set(CMAKE_LINKER_FLAGS_ASAN "${CMAKE_LINKER_FLAGS_DEBUG} ${CMAKE_LINKER_FLAGS_ASAN} -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined")
set(CMAKE_LINKER_FLAGS_PROF "${CMAKE_LINKER_FLAGS_RELEASE} ${CMAKE_LINKER_FLAGS_PROF}")

list(APPEND KRIT_LIBS)
list(APPEND KRIT_STATIC_LIBS)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    find_package(OpenGL REQUIRED)
endif()

if(KRIT_ENABLE_TOOLS)
    message(STATUS "tools enabled")
    add_definitions(-DKRIT_ENABLE_TOOLS)
    file(GLOB_RECURSE TOOLS_FILES CONFIGURE_DEPENDS ${KRIT_DIR}/imgui/*.cpp)
    include_directories(${KRIT_DIR}/imgui)
endif()

file(GLOB KRIT_SRC_FILES CONFIGURE_DEPENDS
    ${KRIT_DIR}/src/krit/*.cpp
    ${KRIT_DIR}/src/krit/asset/*.cpp
    ${KRIT_DIR}/src/krit/behavior/*.cpp
    ${KRIT_DIR}/src/krit/data/*.cpp
    ${KRIT_DIR}/src/krit/ecs/*.cpp
    ${KRIT_DIR}/src/krit/input/*.cpp
    ${KRIT_DIR}/src/krit/io/*.cpp
    ${KRIT_DIR}/src/krit/l12n/*.cpp
    ${KRIT_DIR}/src/krit/math/*.cpp
    ${KRIT_DIR}/src/krit/net/*.cpp
    ${KRIT_DIR}/src/krit/platform/*.cpp
    ${KRIT_DIR}/src/krit/render/*.cpp
    ${KRIT_DIR}/src/krit/renderer/*.cpp
    ${KRIT_DIR}/src/krit/sound/*.cpp
    ${KRIT_DIR}/src/krit/sprites/*.cpp
    ${KRIT_DIR}/src/krit/state/*.cpp
    ${KRIT_DIR}/src/krit/utils/*.cpp
)
file(GLOB_RECURSE KRIT_THIRDPARTY_FILES CONFIGURE_DEPENDS ${KRIT_DIR}/spine-runtimes/spine-cpp/spine-cpp/src/spine/*.cpp)

file(GLOB_RECURSE KRIT_SCRIPT_FILES CONFIGURE_DEPENDS
    ${KRIT_DIR}/src/krit/script/*.cpp
)
list(APPEND KRIT_SRC_FILES ${KRIT_SCRIPT_FILES})

if(KRIT_ENABLE_TOOLS)
    file(GLOB_RECURSE KRIT_EDITOR_FILES CONFIGURE_DEPENDS
        ${KRIT_DIR}/src/krit/editor/*.cpp
    )
    list(APPEND KRIT_SRC_FILES ${KRIT_EDITOR_FILES})
endif()

if(BUILD_WINDOWS)
    include(${KRIT_DIR}/toolchain/windows.cmake)
    add_definitions(-DKRIT_WINDOWS)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    include(${KRIT_DIR}/toolchain/emscripten.cmake)
    add_definitions(-DKRIT_EMSCRIPTEN)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    include(${KRIT_DIR}/toolchain/linux.cmake)
    add_definitions(-DKRIT_LINUX)
else()
    message(FATAL_ERROR "Unrecognized platform")
endif()

if(KRIT_BACKEND_IO STREQUAL "File")
    list(APPEND KRIT_SRC_FILES
        ${KRIT_DIR}/backend/io/IoFileHelper.cpp
        ${KRIT_DIR}/backend/io/IoFile.cpp
    )
elseif(KRIT_BACKEND_IO STREQUAL "Zip")
    list(APPEND KRIT_SRC_FILES
        ${KRIT_DIR}/backend/io/IoFileHelper.cpp
        ${KRIT_DIR}/backend/io/IoZip.cpp
    )
else()
    message(FATAL_ERROR "unrecognized IO backend: ${KRIT_BACKEND_IO}")
endif()

if(KRIT_BACKEND_PLATFORM STREQUAL "Desktop")
    list(APPEND KRIT_SRC_FILES
        ${KRIT_DIR}/backend/platform/PlatformDesktop.cpp
    )
elseif(KRIT_BACKEND_PLATFORM STREQUAL "Web")
    list(APPEND KRIT_SRC_FILES
        ${KRIT_DIR}/backend/platform/PlatformWeb.cpp
    )
else()
    message(FATAL_ERROR "unrecognized platform backend: ${KRIT_BACKEND_PLATFORM}")
endif()

if(KRIT_BACKEND_RENDERER STREQUAL "Gl")
    list(APPEND KRIT_SRC_FILES
        ${KRIT_DIR}/backend/renderer/RendererGl.cpp
    )
else()
    message(FATAL_ERROR "unrecognized renderer backend: ${KRIT_BACKEND_RENDERER}")
endif()

if(KRIT_BACKEND_NET STREQUAL "Curl")
    list(APPEND KRIT_SRC_FILES
        ${KRIT_DIR}/backend/net/NetCurl.cpp
    )
elseif(KRIT_BACKEND_NET STREQUAL "Fetch")
    list(APPEND KRIT_SRC_FILES
        ${KRIT_DIR}/backend/net/NetFetch.cpp
    )
else()
    message(FATAL_ERROR "unrecognized net backend: ${KRIT_BACKEND_NET}")
endif()

include_directories(${KRIT_INCLUDE_PATHS})
include_directories(${KRIT_DIR}/backend)
include_directories(${KRIT_DIR}/src)
include_directories(${KRIT_DIR}/spine-runtimes/spine-cpp/spine-cpp/include)
include_directories(${KRIT_DIR}/quickjs)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/..)

# string(STRIP ${SDL2_LIBRARIES} SDL2_LIBRARIES)
file(GLOB_RECURSE KRIT_GENERATED_FILES
    CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/krit/script/ScriptBridge.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/krit/script/ScriptClassGenerated.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/krit/script/ScriptClass.*.cpp
)

if(CMAKE_BUILD_TYPE MATCHES PROF)
    message(STATUS "profiling enabled")
    list(APPEND KRIT_SRC_FILES ${KRIT_DIR}/tracy/TracyClient.cpp)
endif()

add_definitions(-DDUMP_LEAKS)

list(APPEND KRIT_SRC_FILES
    ${KRIT_DIR}/quickjs/quickjs.h
    ${KRIT_DIR}/quickjs/quickjs-libc.h
    ${KRIT_DIR}/quickjs/quickjs.c
    ${KRIT_DIR}/quickjs/libregexp.c
    ${KRIT_DIR}/quickjs/libunicode.c
    ${KRIT_DIR}/quickjs/libbf.c
    ${KRIT_DIR}/quickjs/cutils.c
    ${KRIT_DIR}/quickjs/quickjs-libc.c
)

add_library(${PROJECT_NAME} OBJECT
    ${KRIT_GENERATED_FILES}
    ${KRIT_SRC_FILES}
    ${KRIT_THIRDPARTY_FILES}
    ${TOOLS_FILES}
)

if(KRIT_ENABLE_THREADS)
    add_definitions(-DKRIT_ENABLE_THREADS)
endif()

if(KRIT_ENABLE_MULTISAMPLING)
    add_definitions(-DKRIT_ENABLE_MULTISAMPLING)
endif()

include_directories(${KRIT_DIR}/src)
include_directories(${KRIT_DIR}/spine-runtimes/spine-cpp/spine-cpp/include)
include_directories(${KRIT_DIR}/imgui)
include_directories(${KRIT_DIR}/quickjs)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

message(STATUS "toolchain library path:${TOOLCHAIN_LIB_PATH} prefixes:${CMAKE_FIND_LIBRARY_PREFIXES} suffixes:${CMAKE_FIND_LIBRARY_SUFFIXES}")

message("==> ${KRIT_STATIC_LIBS}")

if(KRIT_ENABLE_THREADS)
    list(APPEND KRIT_STATIC_LIBS "-lpthread")
endif()

if(KRIT_ENABLE_TOOLS)
    find_library(NFD_LIB NAMES nfd HINTS "${TOOLCHAIN_LIB_PATH}" CMAKE_FIND_ROOT_PATH_BOTH)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
    list(APPEND KRIT_STATIC_LIBS ${NFD_LIB})
    list(APPEND KRIT_LIBS ${GTK3_LIBRARIES})
endif()

target_link_libraries(${PROJECT_NAME}
    PUBLIC
    ${KRIT_LIBS}
    ${STATIC}
    ${KRIT_STATIC_LIBS}
    "-Wl,--gc-sections"
    "-static-libgcc"
)
target_include_directories(${PROJECT_NAME} PRIVATE ${FREETYPE_INCLUDE_DIRS})

if(KRIT_BUILD_ASSETS)
    add_custom_target(
        copy-assets ALL
        COMMAND
        ${KRIT_DIR}/tools/update_assets.sh ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KRIT_DIR}
        BYPRODUCTS assets.zip ${CMAKE_CURRENT_BINARY_DIR}/images.yaml
    )
endif()

file(GLOB_RECURSE KRIT_EDITOR_FILES CONFIGURE_DEPENDS
    ${KRIT_DIR}/tools/tsbindgen/src/*.ts
)

add_custom_command(
    OUTPUT ${KRIT_DIR}/tools/tsbindgen/dist/index.js
    DEPENDS ${BINDGEN_SRC_FILES}
    COMMAND cd ${KRIT_DIR}/tools/tsbindgen && npm install && npm run build
)

function(tsbindgen BRIDGE_SRC_DIR PREFIX NAMESPACE)
    message("tsbindgen: ${BRIDGE_SRC_DIR}")
    file(GLOB_RECURSE BRIDGE_TYPE_DEFS CONFIGURE_DEPENDS ${BRIDGE_SRC_DIR}/*.d.ts)
    set(BRIDGE_OUTPUTS "")

    foreach(BRIDGE ${BRIDGE_TYPE_DEFS})
        file(RELATIVE_PATH BRIDGE_RELATIVE ${BRIDGE_SRC_DIR} ${BRIDGE})
        set_directory_properties(PROPERTIES CMAKE_CONFIGURE_DEPENDS ${BRIDGE})
        list(APPEND BRIDGE_OUTPUTS "${CMAKE_CURRENT_BINARY_DIR}/${PREFIX}/${BRIDGE_RELATIVE}.ScriptClass.cpp")
    endforeach()

    get_property(CURRENT_BRIDGE_NAMESPACES GLOBAL PROPERTY BRIDGE_NAMESPACES)
    message("cd ${KRIT_DIR}/tools/tsbindgen && node dist/index.js -n ${NAMESPACE}:${BRIDGE_SRC_DIR} ${CURRENT_BRIDGE_NAMESPACES} -s ${BRIDGE_SRC_DIR} -o ${CMAKE_CURRENT_BINARY_DIR}/${PREFIX}")
    add_custom_command(
        OUTPUT ${BRIDGE_OUTPUTS}
        DEPENDS ${BRIDGE_TYPE_DEFS} ${KRIT_DIR}/tools/tsbindgen/dist/index.js ${KRIT_DIR}/tools/tsbindgen/templates/ScriptClass.cpp.njk
        COMMAND cd ${KRIT_DIR}/tools/tsbindgen && node dist/index.js -n ${NAMESPACE}:${BRIDGE_SRC_DIR} ${CURRENT_BRIDGE_NAMESPACES} -s ${BRIDGE_SRC_DIR} -o "${CMAKE_CURRENT_BINARY_DIR}/${PREFIX}"
    )

    target_sources(${PROJECT_NAME} PRIVATE ${BRIDGE_OUTPUTS})

    set_property(GLOBAL PROPERTY BRIDGE_NAMESPACES "${CURRENT_BRIDGE_NAMESPACES}" "-n" "${NAMESPACE}:${BRIDGE_SRC_DIR}")
endfunction()

tsbindgen(${KRIT_DIR}/src generated/krit krit)
