set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR AMD64)

include(CMakeForceCompiler)
set(CMAKE_C_COMPILER i686-w64-mingw32-gcc-posix)
set(CMAKE_CXX_COMPILER i686-w64-mingw32-g++-posix)
set(CMAKE_RC_COMPILER i686-w64-mingw32-windres)
set(CMAKE_RC_FLAGS -DGCC_WINDRES)
set(CMAKE_LINKER i686-w64-mingw32-ldd)
set(CMAKE_CFLAGS "${CMAKE_CFLAGS} -static -static-libgcc -static-libstdc++ -gcoff -DWINVER=0x0600 -DNTDDI_VERSION=0x06000000 -D_WIN32_WINNT=0x0600")
set(CMAKE_CPPFLAGS "${CMAKE_CPPFLAGS} -static -static-libgcc -static-libstdc++ -DWINVER=0x0600 -DNTDDI_VERSION=0x06000000 -D_WIN32_WINNT=0x0600")
set(CMAKE_CXXFLAGS "${CMAKE_CXXFLAGS} -static -static-libgcc -static-libstdc++ -DWINVER=0x0600 -DNTDDI_VERSION=0x06000000 -D_WIN32_WINNT=0x0600")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")

set(CMAKE_FIND_ROOT_PATH
  /usr/i686-w64-mingw32
  $ENV{PWD}/download-mingw-rpm/usr/i686-w64-mingw32/sys-root/mingw
  $ENV{PWD}/mingw-install
  $ENV{HOME}/mingw-install
)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

link_directories(${CMAKE_SOURCE_DIR}/krit/toolchain/windows/bin)
