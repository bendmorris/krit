project(krit)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(OpenAL REQUIRED)

file(GLOB KRIT_SRC_FILES
    src/krit/*.cpp
    src/krit/**/*.cpp
    spine-runtimes/spine-cpp/spine-cpp/src/spine/*.cpp
)
include_directories(src)
include_directories(spine-runtimes/spine-cpp/spine-cpp/include)

file(GLOB KRIT_BACKEND_SRC_FILES
    backend/sdl/krit/*.cpp
    backend/sdl/krit/**/*.cpp
)
include_directories(backend/sdl)

string(STRIP ${SDL2_LIBRARIES} SDL2_LIBRARIES)

add_library(${PROJECT_NAME} STATIC ${KRIT_SRC_FILES} ${KRIT_BACKEND_SRC_FILES})
target_link_libraries(${PROJECT_NAME}
    PUBLIC
    ${SDL2_LIBRARIES}
    ${OPENGL_LIBRARIES}
    "-lyaml"
    "-lexpat"
    "-lopenal"
    "-lSDL2_image"
    "-Wl,--gc-sections"
)
