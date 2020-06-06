project(krit)

find_package(SDL2 REQUIRED)
#find_package(SDL2_image REQUIRED)
find_package(OpenGL REQUIRED)
find_package(OpenAL REQUIRED)
find_package(GLEW REQUIRED)
#find_package(libzip REQUIRED)

add_definitions(
    "-DIMGUI_IMPL_OPENGL_LOADER_GLEW"
)

file(GLOB KRIT_SRC_FILES
    CONFIGURE_DEPENDS
    src/krit/*.cpp
    src/krit/**/*.cpp
    spine-runtimes/spine-cpp/spine-cpp/src/spine/*.cpp
    imgui/*.cpp
)
include_directories(src)
include_directories(spine-runtimes/spine-cpp/spine-cpp/include)
include_directories(/usr/include/SDL2)
include_directories(imgui)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../)

string(STRIP ${SDL2_LIBRARIES} SDL2_LIBRARIES)

add_library(${PROJECT_NAME} STATIC
    ${CMAKE_CURRENT_BINARY_DIR}/Assets.cpp
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
)

add_custom_target(
    compile-assets ALL
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/tools/asset_registry/asset_registry.py --input ${CMAKE_SOURCE_DIR}/assets.yaml --output-dir ${CMAKE_CURRENT_BINARY_DIR}
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/Assets.h ${CMAKE_CURRENT_BINARY_DIR}/Assets.cpp
)
add_dependencies(${PROJECT_NAME} compile-assets)

add_custom_target(
    copy-assets ALL
    COMMAND
        rsync --delete -a ${CMAKE_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR} &&
        cd ${CMAKE_BINARY_DIR} && zip -r assets.zip assets
    BYPRODUCTS assets.zip
)
add_dependencies(compile-assets copy-assets)
