export PREFIX=$(shell pwd)
export C_INCLUDE_PATH=$(shell pwd)/include
export CPLUS_INCLUDE_PATH=$(shell pwd)/include
export CFLAGS=-fno-finite-math-only -I${PREFIX}/include -static -O3
export CXXFLAGS=-I${PREFIX}/include -static -O3
export LDFLAGS=-L${PREFIX}/lib -L${PREFIX}/lib -static
export PKG_CONFIG_PATH=$(shell pwd)/lib/pkgconfig
export BUILD=x86_64-pc-linux-gnu
export HOST=x86_64-pc-linux-gnu
OPENAL_LIB_NAME=libopenal.a

include ../build-toolchain.make
