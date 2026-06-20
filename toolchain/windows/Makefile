export BUILD=x86_64-pc-linux-gnu
export HOST=i686-w64-mingw32
export TOOLCHAIN_PREFIX=i686-w64-mingw32-
export PREFIX=$(shell pwd)
export AR=${TOOLCHAIN_PREFIX}ar
export CC=${TOOLCHAIN_PREFIX}gcc
export CXX=${TOOLCHAIN_PREFIX}g++
export RANLIB=${TOOLCHAIN_PREFIX}ranlib
export WINDRES=${TOOLCHAIN_PREFIX}windres
export STRIP=${TOOLCHAIN_PREFIX}strip
export LDFLAGS=-L$(shell pwd)/lib -lpthread -lws2_32 -lcrypt32 -lavrt -static -static-libgcc -static-libstdc++
export C_INCLUDE_PATH=$(shell pwd)/include
export CPLUS_INCLUDE_PATH=$(shell pwd)/include
export CFLAGS=-I$(shell pwd)/include -DFLAC__NO_DLL -O3 -DWINVER=0x0600 -DNTDDI_VERSION=0x06000000 -D_WIN32_WINNT=0x0600
export CPPFLAGS=-I$(shell pwd)/include -O3 -DWINVER=0x0600 -DNTDDI_VERSION=0x06000000 -D_WIN32_WINNT=0x0600
export CXXFLAGS=-I$(shell pwd)/include -O3 -DWINVER=0x0600 -DNTDDI_VERSION=0x06000000 -D_WIN32_WINNT=0x0600
export PKG_CONFIG=i686-w64-mingw32-pkg-config
export PKG_CONFIG_PATH=$(shell pwd)/lib/pkgconfig
export LIBS=-lws2_32 -lcrypt32
export CMAKE_TOOLCHAIN=-DCMAKE_TOOLCHAIN_FILE=$(shell pwd)/../windows.toolchain.cmake
export MESON_CROSS=--cross-file $(shell pwd)/i686-mingw-w64.meson.ini
export SSL_CONFIGURE=mingw
export MESON_CFLAGS="-static"
export MESON_LDFLAGS=-static -static-libgcc -static-libstdc++

OPENAL_LIB_NAME=lib/libOpenAL32.a

include ../build-toolchain.make

all: dlfcn

# dlfcn: needed for -ldl which is required at minimum for curl
dlfcn: lib/libdl.a
build/dlfcn-win32-1.3.1.tar.gz:
	curl -L https://github.com/dlfcn-win32/dlfcn-win32/archive/refs/tags/v1.3.1.tar.gz -o $@
build/dlfcn-win32-1.3.1: build/dlfcn-win32-1.3.1.tar.gz
	tar xzf $< -C build
lib/libdl.a: build/dlfcn-win32-1.3.1
	cd $< && ./configure --cross-prefix=$$TOOLCHAIN_PREFIX --enable-static --disable-shared --prefix=$$PREFIX --libdir=$$PREFIX/lib --incdir=$$PREFIX/include && make -j8 && make install
