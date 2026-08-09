cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

set(KRIT_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(KRIT_DIR "${CMAKE_CURRENT_LIST_DIR}" PARENT_SCOPE)

add_library(krit INTERFACE)

cmake_policy(SET CMP0072 NEW)
cmake_policy(SET CMP0077 NEW)
cmake_policy(SET CMP0135 NEW)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_COLOR_DIAGNOSTICS ON)

option(BUILD_WINDOWS "Cross-compile for Windows using mingw-w64" OFF)
option(KRIT_ENABLE_AUDIO "Enable audio system" ON)
option(KRIT_ENABLE_BEHAVIORTREE "Enable behavior trees" ON)
option(KRIT_ENABLE_CURSORS "Enable audio system" ON)
option(KRIT_ENABLE_ECS "Enable entity component system" ON)
option(KRIT_ENABLE_MULTISAMPLING "Whether to use multisampling for framebuffers" ON)
option(KRIT_ENABLE_NET "Enable networking" ON)
option(KRIT_ENABLE_SCRIPT "Enable scriptengine" ON)
option(KRIT_ENABLE_SPINE "Enable spine sprites" ON)
option(KRIT_ENABLE_SQLITE "Enable sqlite DB support" ON)
option(KRIT_ENABLE_TEXT "Enable sqlite DB support" ON)
option(KRIT_ENABLE_THREADS "Whether task managers should use multiple threads" ON)
option(KRIT_ENABLE_TOOLS "Include imgui and devtools functionality" OFF)
option(KRIT_ENABLE_TRACY "Enable profiling hooks for Tracy" OFF)
option(KRIT_MAIN "Inlude the krit main.cpp file" ON)
option(KRIT_USE_GLEW "Whether to use glew instead of direct GL" ON)

if(KRIT_ENABLE_AUDIO)
    target_compile_definitions(krit INTERFACE -DKRIT_ENABLE_AUDIO)
endif()
if(KRIT_ENABLE_BEHAVIORTREE)
    target_compile_definitions(krit INTERFACE -DKRIT_ENABLE_BEHAVIORTREE)
endif()
if(KRIT_ENABLE_CURSORS)
    target_compile_definitions(krit INTERFACE -DKRIT_ENABLE_CURSORS)
endif()
if(KRIT_ENABLE_ECS)
    target_compile_definitions(krit INTERFACE -DKRIT_ENABLE_ECS)
endif()
if(KRIT_ENABLE_MULTISAMPLING)
    target_compile_definitions(krit INTERFACE -DKRIT_ENABLE_MULTISAMPLING)
endif()
if(KRIT_ENABLE_NET)
    target_compile_definitions(krit INTERFACE -DKRIT_ENABLE_NET)
endif()
if(KRIT_ENABLE_SCRIPT)
    target_compile_definitions(krit INTERFACE -DKRIT_ENABLE_SCRIPT)
endif()
if(KRIT_ENABLE_SPINE)
    target_compile_definitions(krit INTERFACE -DKRIT_ENABLE_SPINE)
endif()
if(KRIT_ENABLE_SQLITE)
    target_compile_definitions(krit INTERFACE -DKRIT_ENABLE_SQLITE)
endif()
if(KRIT_ENABLE_TEXT)
    target_compile_definitions(krit INTERFACE -DKRIT_ENABLE_TEXT)
endif()
if(KRIT_ENABLE_THREADS)
    target_compile_definitions(krit INTERFACE -DKRIT_ENABLE_THREADS)
endif()
if(KRIT_ENABLE_TOOLS)
    target_compile_definitions(krit INTERFACE -DKRIT_ENABLE_TOOLS)
endif()
if(KRIT_ENABLE_TRACY)
    target_compile_definitions(krit INTERFACE -DKRIT_ENABLE_TRACY)
endif()
if(KRIT_MAIN)
    target_compile_definitions(krit INTERFACE -DKRIT_MAIN)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(default_dump_leaks OFF)
else()
    set(default_dump_leaks ON)
endif()
option(KRIT_QUICKJS_DUMP_LEAKS "Dump any JS leaks on exit" ${default_dump_leaks})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -MMD -MF .obj/.d -Wno-array-bounds -Wno-format-truncation -D_GNU_SOURCE -fno-omit-frame-pointer -fasynchronous-unwind-tables")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE} -O2")
set(CMAKE_C_FLAGS_PROF "${CMAKE_C_FLAGS_RELEASE} ${CMAKE_C_FLAGS_PROF}")
set(CMAKE_CXX_COMPILER_LAUNCHER ccache)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-parentheses -Wno-unused-result -Wno-c99-designator -Wall -ffunction-sections -fdata-sections -Os -fno-rtti -falign-functions=1 -fmerge-all-constants -std=c++17")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -ffunction-sections -fdata-sections -DKRIT_RELEASE")
set(CMAKE_CXX_FLAGS_PROF "${CMAKE_CXX_FLAGS_RELEASE} ${CMAKE_CXX_FLAGS_PROF} -Wno-sometimes-uninitialized -g")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_ASAN "${CMAKE_CXX_FLAGS_DEBUG} ${CMAKE_CXX_FLAGS_ASAN} -fsanitize=address -fsanitize=undefined")

set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -rdynamic")
set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -lmcheck")
set(CMAKE_LINKER_FLAGS_ASAN "${CMAKE_LINKER_FLAGS_DEBUG} ${CMAKE_LINKER_FLAGS_ASAN} -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined")
set(CMAKE_LINKER_FLAGS_PROF "${CMAKE_LINKER_FLAGS_RELEASE} ${CMAKE_LINKER_FLAGS_PROF}")

list(APPEND KRIT_LIBS)
list(APPEND KRIT_STATIC_LIBS)

if(KRIT_ENABLE_TOOLS)
    message(STATUS "tools enabled")
    target_compile_definitions(krit INTERFACE -DKRIT_ENABLE_TOOLS)
    file(GLOB_RECURSE TOOLS_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/imgui/*.cpp)
    target_include_directories(krit INTERFACE ${CMAKE_CURRENT_LIST_DIR}/imgui)
endif()

file(GLOB_RECURSE KRIT_SRC_FILES CONFIGURE_DEPENDS src/**/*.cpp)

if(KRIT_ENABLE_SPINE)
    file(GLOB_RECURSE KRIT_THIRDPARTY_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/spine-runtimes/spine-cpp/spine-cpp/src/spine/*.cpp)
endif()

if(KRIT_ENABLE_TRACY)
    target_compile_definitions(krit INTERFACE -DTRACY_ENABLE -DTRACY_NO_CRASH_HANDLER -DTRACY_ON_DEMAND)
    list(APPEND TRACY_FILES
        ${CMAKE_CURRENT_LIST_DIR}/tracy/public/TracyClient.cpp
    )
    list(APPEND KRIT_SRC_FILES ${TRACY_FILES})
    include_directories(${CMAKE_CURRENT_LIST_DIR}/tracy/public/tracy)
    set_source_files_properties(${TRACY_FILES} PROPERTIES
        COMPILE_FLAGS "-w" # GCC/Clang: Suppress all warnings
    )
    # list(APPEND KRIT_SRC_FILES ${CMAKE_CURRENT_LIST_DIR}/tracy/TracyClient.cpp)
    # include_directories(${CMAKE_CURRENT_LIST_DIR}/tracy)
endif()

if(KRIT_ENABLE_TOOLS)
    file(GLOB_RECURSE KRIT_EDITOR_FILES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_LIST_DIR}/src/krit/editor/*.cpp
    )
    list(APPEND KRIT_SRC_FILES ${KRIT_EDITOR_FILES})
endif()

message(STATUS "system: ${CMAKE_SYSTEM_NAME}")
if(BUILD_WINDOWS)
    include(${CMAKE_CURRENT_LIST_DIR}/toolchain/windows.cmake)
    target_compile_definitions(krit INTERFACE -DKRIT_WINDOWS)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    include(${CMAKE_CURRENT_LIST_DIR}/toolchain/emscripten.cmake)
    target_compile_definitions(krit INTERFACE -DKRIT_EMSCRIPTEN)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    include(${CMAKE_CURRENT_LIST_DIR}/toolchain/linux.cmake)
    target_compile_definitions(krit INTERFACE -DKRIT_LINUX)
else()
    message(FATAL_ERROR "Unrecognized platform")
endif()

if(KRIT_USE_GLEW)
    message(STATUS "using glew")
    target_compile_definitions(krit INTERFACE -DKRIT_USE_GLEW -DGLEW_STATIC)
    list(APPEND KRIT_SRC_FILES ${CMAKE_CURRENT_LIST_DIR}/src/GL/glew.c)
    set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/src/GL/glew.c
        PROPERTIES COMPILE_OPTIONS "-Wno-error"
    )
endif()

if(KRIT_BACKEND_PLATFORM STREQUAL "Desktop")
    list(APPEND KRIT_SRC_FILES
        ${CMAKE_CURRENT_LIST_DIR}/backend/platform/PlatformDesktop.cpp
    )
elseif(KRIT_BACKEND_PLATFORM STREQUAL "Web")
    list(APPEND KRIT_SRC_FILES
        ${CMAKE_CURRENT_LIST_DIR}/backend/platform/PlatformWeb.cpp
    )
else()
    message(FATAL_ERROR "unrecognized platform backend: ${KRIT_BACKEND_PLATFORM}")
endif()

if(KRIT_BACKEND_RENDERER STREQUAL "Gl")
    list(APPEND KRIT_SRC_FILES
        ${CMAKE_CURRENT_LIST_DIR}/backend/renderer/RendererGl.cpp
    )
else()
    message(FATAL_ERROR "unrecognized renderer backend: ${KRIT_BACKEND_RENDERER}")
endif()

if (KRIT_ENABLE_NET)
    if(KRIT_BACKEND_NET STREQUAL "Curl")
        list(APPEND KRIT_SRC_FILES
            ${CMAKE_CURRENT_LIST_DIR}/backend/net/NetCurl.cpp
        )
    elseif(KRIT_BACKEND_NET STREQUAL "Fetch")
        list(APPEND KRIT_SRC_FILES
            ${CMAKE_CURRENT_LIST_DIR}/backend/net/NetFetch.cpp
        )
    else()
        message(FATAL_ERROR "unrecognized net backend: ${KRIT_BACKEND_NET}")
    endif()
endif()

target_include_directories(krit INTERFACE ${KRIT_INCLUDE_PATHS})
target_include_directories(krit INTERFACE ${CMAKE_CURRENT_LIST_DIR}/backend)
target_include_directories(krit INTERFACE ${CMAKE_CURRENT_LIST_DIR}/src)
if(KRIT_ENABLE_SPINE)
    target_include_directories(krit INTERFACE ${CMAKE_CURRENT_LIST_DIR}/spine-runtimes/spine-cpp/spine-cpp/include)
endif()

target_sources(krit PUBLIC
    ${KRIT_GENERATED_FILES}
    ${KRIT_SRC_FILES}
    ${KRIT_THIRDPARTY_FILES}
    ${TOOLS_FILES}
)

if(KRIT_ENABLE_THREADS)
    target_compile_definitions(krit INTERFACE -DKRIT_ENABLE_THREADS)
    list(APPEND KRIT_STATIC_LIBS "-lpthread")
endif()

if(KRIT_ENABLE_MULTISAMPLING)
    target_compile_definitions(krit INTERFACE -DKRIT_ENABLE_MULTISAMPLING)
endif()

message("==> static libs: ${KRIT_STATIC_LIBS}")

if(KRIT_ENABLE_TOOLS)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
    list(APPEND KRIT_LIBS ${GTK3_LIBRARIES})
endif()

target_link_libraries(krit
    INTERFACE
    "-Wl,-Bdynamic"
    ${KRIT_LIBS}
    ${KRIT_STATIC}
    ${KRIT_STATIC_LIBS}
    ${KRIT_PLATFORM_STATIC_LIBS}
    "-Wl,--gc-sections"
)
if(KRIT_ENABLE_TEXT)
    target_include_directories(krit INTERFACE ${FREETYPE_INCLUDE_DIRS})
endif()

if(KRIT_ENABLE_SCRIPT)
    target_include_directories(krit INTERFACE ${CMAKE_CURRENT_LIST_DIR}/quickjs)
    target_include_directories(krit INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/..)

    file(GLOB_RECURSE KRIT_GENERATED_FILES
        CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_BINARY_DIR}/krit/script/ScriptBridge.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/krit/script/ScriptClass.*.cpp
    )

    if(KRIT_QUICKJS_DUMP_LEAKS)
        target_compile_definitions(krit INTERFACE -DDUMP_LEAKS)
    endif()

    target_sources(krit INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/quickjs/quickjs.h
        ${CMAKE_CURRENT_LIST_DIR}/quickjs/quickjs.c
        ${CMAKE_CURRENT_LIST_DIR}/quickjs/libregexp.c
        ${CMAKE_CURRENT_LIST_DIR}/quickjs/libunicode.c
        ${CMAKE_CURRENT_LIST_DIR}/quickjs/cutils.c
        ${CMAKE_CURRENT_LIST_DIR}/quickjs/dtoa.c
    )
    target_compile_definitions(krit INTERFACE "-DCONFIG_VERSION=\"2021-03-27\"")
    target_link_options(krit INTERFACE "-Wl,--trace-symbol=my_mystery_symbol")

    file(GLOB_RECURSE BINDGEN_SRC_FILES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_LIST_DIR}/tools/tsbindgen/src/*.ts
    )

    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_LIST_DIR}/tools/tsbindgen/dist/index.js
        DEPENDS ${BINDGEN_SRC_FILES}
        COMMAND cd ${CMAKE_CURRENT_LIST_DIR}/tools/tsbindgen && npm install && npm run build
    )

    set(KRIT_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")

    function(tsbindgen TARGET BRIDGE_SRC_DIR PREFIX NAMESPACE)
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

        target_sources(${TARGET} PUBLIC ${BRIDGE_OUTPUTS})

        set_property(GLOBAL PROPERTY BRIDGE_NAMESPACES "${CURRENT_BRIDGE_NAMESPACES}" "-n" "${NAMESPACE}:${BRIDGE_SRC_DIR}")
    endfunction()

    tsbindgen(krit ${CMAKE_CURRENT_LIST_DIR}/src generated/krit krit)
endif()

if(KRIT_ENABLE_TRACY AND BUILD_WINDOWS)
    target_link_libraries(krit INTERFACE ws2_32 dbghelp)
endif()

if(KRIT_BACKEND_PLATFORM STREQUAL "Desktop")
    message("including backward")
    target_compile_definitions(krit INTERFACE -DKRIT_BACKWARD)
    target_include_directories(krit INTERFACE ${CMAKE_CURRENT_LIST_DIR}/backward-cpp)
endif()

set_target_properties(krit PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)
