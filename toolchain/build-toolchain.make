.PHONY: all clean curl curses flac freetype harfbuzz jpeg libedit ogg openal openssl opus png sdl sdl_image sndfile sqlite vorbis yaml zip zlib
all: curl flac flatbuffers freetype harfbuzz jpeg ogg openal openssl opus png sdl sdl_image sndfile sqlite vorbis yaml zip zlib
clean:
	rm -rf lib/* `ls -d build/*/`

.MKDIR: $(shell mkdir -p build)

# flatbuffers
FLATBUFFERS_VERSION:=1.12.1
flatbuffers: lib/libflatbuffers.a
build/flatbuffers-$(FLATBUFFERS_VERSION).tar.gz:
	curl -L https://github.com/google/flatbuffers/archive/refs/tags/v$(FLATBUFFERS_VERSION).tar.gz -o $@
build/flatbuffers-$(FLATBUFFERS_VERSION): build/flatbuffers-$(FLATBUFFERS_VERSION).tar.gz
	tar xf $< -C build
lib/libflatbuffers.a: build/flatbuffers-$(FLATBUFFERS_VERSION)
	mkdir -p build/flatbuffers-$(FLATBUFFERS_VERSION)/build
	cd build/flatbuffers-$(FLATBUFFERS_VERSION)/build && cmake -DCMAKE_BUILD_TYPE=Release -G"Unix Makefiles" $$CMAKE_TOOLCHAIN -DCMAKE_INSTALL_PREFIX=$$PREFIX -D FLATBUFFERS_BUILD_TESTS=OFF -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_CXX_STANDARD=17 .. && make flatbuffers flatc -j8 && make install

# freetype
FREETYPE_VERSION:=2.12.1
freetype: lib/libfreetype.a
build/freetype-${FREETYPE_VERSION}.tar.xz:
	curl -L https://download.savannah.gnu.org/releases/freetype/freetype-${FREETYPE_VERSION}.tar.xz -o $@
build/freetype-${FREETYPE_VERSION}: build/freetype-${FREETYPE_VERSION}.tar.xz
	tar xf $< -C build
lib/libfreetype.a: build/freetype-${FREETYPE_VERSION} lib/libz.a include/harfbuzz/hb-ft.h
	cd $< && ./configure --build=$$BUILD --host=$$HOST --enable-static --disable-shared --without-bzip2 --without-brotli --without-harfbuzz --prefix=$$PREFIX && make -j8 && make install

# harfbuzz
HARFBUZZ_VERSION:=14.2.1
harfbuzz: lib/libharfbuzz.a
build/harfbuzz-${HARFBUZZ_VERSION}.tar.xz:
	curl -L https://github.com/harfbuzz/harfbuzz/releases/download/${HARFBUZZ_VERSION}/harfbuzz-${HARFBUZZ_VERSION}.tar.xz -o $@
build/harfbuzz-${HARFBUZZ_VERSION}: build/harfbuzz-${HARFBUZZ_VERSION}.tar.xz
	tar xf $< -C build
# harfbuzz and freetype have a circular dependency; freetype needs to be able to find this to build
include/harfbuzz/hb-ft.h: build/harfbuzz-${HARFBUZZ_VERSION}
	mkdir -p `dirname $@` && cp build/harfbuzz-${HARFBUZZ_VERSION}/src/hb-ft.h $@
lib/libharfbuzz.a: build/harfbuzz-${HARFBUZZ_VERSION} lib/libfreetype.a include/harfbuzz/hb-ft.h
	cd $< && cmake -B build -DCMAKE_BUILD_TYPE=Release -G"Unix Makefiles" $$CMAKE_TOOLCHAIN -DCMAKE_INSTALL_PREFIX=$$PREFIX -DCMAKE_POSITION_INDEPENDENT_CODE=ON -D BUILD_SHARED_LIBS=OFF -D HB_HAVE_FREETYPE=Off && cmake --build build -- -j8 && cmake --build build -- install

# libedit
libedit: lib/libedit.a
build/libedit-20240808-3.1.tar.gz:
	curl -L https://www.thrysoee.dk/editline/libedit-20240808-3.1.tar.gz -o $@
build/libedit-20240808-3.1: build/libedit-20240808-3.1.tar.gz
	tar xzf $< -C build
lib/libedit.a: build/libedit-20240808-3.1 lib/libncurses.a
	cd $< && ./configure --build=$$BUILD --host=$$HOST --enable-static --disable-shared --prefix=$$PREFIX && make -j8 && make install

# openal
openal: $(OPENAL_LIB_NAME)
build/openal-soft-1.24.0.tar.gz:
	curl -L https://github.com/kcat/openal-soft/archive/refs/tags/1.24.0.tar.gz -o $@
build/openal-soft-1.24.0: build/openal-soft-1.24.0.tar.gz
	tar xzf $< -C build
$(OPENAL_LIB_NAME): build/openal-soft-1.24.0 lib/libsndfile.a lib/libz.a
	cd $< && cmake -B build -DCMAKE_BUILD_TYPE=Release -G"Unix Makefiles" $$CMAKE_TOOLCHAIN -DCMAKE_INSTALL_PREFIX=$$PREFIX -DLIBTYPE=STATIC -DALSOFT_UTILS=OFF -DALSOFT_EXAMPLES=OFF -DSNDFILE_LIBRARY=$(shell pwd)/lib/libsndfile.a -DSNDFILE_INCLUDE_DIR=$(shell pwd)/include -DZLIB_LIBRARY=$(shell pwd)/lib/libz.a -DZLIB_INCLUDE_DIR=$(shell pwd)/include && cmake --build build -- -j8 && cmake --build build -- install

# opus
opus: lib/libopus.a
build/opus-1.1.2.tar.gz:
	curl -L https://github.com/xiph/opus/releases/download/v1.1.2/opus-1.1.2.tar.gz -o $@
build/opus-1.1.2: build/opus-1.1.2.tar.gz
	tar xzf $< -C build
lib/libopus.a: build/opus-1.1.2
	cd $< && ./configure --build=$$BUILD --host=$$HOST --enable-static --disable-shared --prefix=$$PREFIX && make -j8 && make install

# sndfile
sndfile: lib/libsndfile.a
build/libsndfile-1.2.2.tar.xz:
	curl -L https://github.com/libsndfile/libsndfile/releases/download/1.2.2/libsndfile-1.2.2.tar.xz -o $@
build/libsndfile-1.2.2: build/libsndfile-1.2.2.tar.xz
	tar xf $< -C build
lib/libsndfile.a: build/libsndfile-1.2.2 lib/libogg.a lib/libvorbis.a lib/libopus.a lib/libFLAC.a
	cd $< && CFLAGS="${CFLAGS} -std=c99" ./configure --build=$$BUILD --host=$$HOST --enable-static --disable-shared --prefix=$$PREFIX  && make -j8 && make install

# vorbis
vorbis: lib/libvorbis.a
build/libvorbis-1.3.7.tar.xz:
	curl -L https://github.com/xiph/vorbis/releases/download/v1.3.7/libvorbis-1.3.7.tar.xz -o $@
build/libvorbis-1.3.7: build/libvorbis-1.3.7.tar.xz
	tar xf $< -C build
lib/libvorbis.a: build/libvorbis-1.3.7 lib/libogg.a
	cd $< && ./configure --build=$$BUILD --host=$$HOST --enable-static --disable-shared --prefix=$$PREFIX && make -j8 && make install
