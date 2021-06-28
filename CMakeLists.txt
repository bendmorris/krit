project(krit)
cmake_policy (SET CMP0072 OLD)

find_package(SDL2 REQUIRED)
#find_package(SDL2_image REQUIRED)
find_package(OpenGL REQUIRED)
find_package(OpenAL REQUIRED)
find_package(GLEW REQUIRED)
#find_package(libzip REQUIRED)
find_package(harfbuzz REQUIRED)
find_package(Freetype REQUIRED)

add_definitions(
    "-DIMGUI_IMPL_OPENGL_LOADER_GLEW"
)

file(GLOB_RECURSE KRIT_SRC_FILES
    CONFIGURE_DEPENDS
    src/krit/*.cpp
    spine-runtimes/spine-cpp/spine-cpp/src/spine/*.cpp
    imgui/*.cpp
)
include_directories(src)
include_directories(spine-runtimes/spine-cpp/spine-cpp/include)
include_directories(/usr/include/SDL2)
include_directories(imgui)
include_directories(quickjs)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../)
include_directories(${KRIT_INCLUDE_PATHS})

string(STRIP ${SDL2_LIBRARIES} SDL2_LIBRARIES)

file(GLOB_RECURSE KRIT_GENERATED_SCRIPT_FILES
    CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/script/ScriptClass.*.cpp
)

add_library(${PROJECT_NAME} STATIC
    ${CMAKE_CURRENT_BINARY_DIR}/Assets.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/script/ScriptBridge.cpp
    ${KRIT_GENERATED_SCRIPT_FILES}
    ${KRIT_SRC_FILES}
)
target_link_libraries(${PROJECT_NAME}
    PUBLIC
    ${SDL2_LIBRARIES}
    ${OPENGL_LIBRARIES}
    "-lyaml"
    "-lexpat"
    "-lopenal"
    "-lSDL2_image"
    "-Wl,--gc-sections"
    "-lGLEW"
    "-lzip"
    "-lfreetype"
    "-lharfbuzz"
    ${CMAKE_CURRENT_SOURCE_DIR}/quickjs/libquickjs.a
)
target_include_directories(${PROJECT_NAME} PRIVATE ${FREETYPE_INCLUDE_DIRS})

add_custom_target(
    quickjs
    COMMAND make libquickjs.a CONFIG_BIGNUM= CONFIG_LTO=n
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/quickjs
)

add_dependencies(${PROJECT_NAME} quickjs)

add_custom_target(
    compile-assets ALL
    COMMAND cd ${CMAKE_BINARY_DIR} && python3 ${CMAKE_CURRENT_SOURCE_DIR}/tools/asset_registry/asset_registry.py --input ${CMAKE_SOURCE_DIR}/assets.yaml --output-dir ${CMAKE_CURRENT_BINARY_DIR}
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/Assets.h ${CMAKE_CURRENT_BINARY_DIR}/Assets.cpp
)
add_dependencies(${PROJECT_NAME} compile-assets)

add_custom_target(
    copy-assets ALL
    COMMAND
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/update_assets.sh ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}
    BYPRODUCTS assets.zip
)
add_dependencies(compile-assets copy-assets)

if(KRIT_SCRIPT_BRIDGE_PATH)
    add_custom_target(
        generate-bridge ALL
        COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR}/tools/bridge_generator && node src/index.js ${KRIT_SCRIPT_BRIDGE_PATH} ${CMAKE_CURRENT_BINARY_DIR}
        BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/script/ScriptClass.h ${CMAKE_CURRENT_BINARY_DIR}/script/ScriptClass.cpp ${CMAKE_CURRENT_BINARY_DIR}/script/ScriptBridge.h ${CMAKE_CURRENT_BINARY_DIR}/script/ScriptBridge.cpp
    )
    add_dependencies(${PROJECT_NAME} generate-bridge)
endif()
