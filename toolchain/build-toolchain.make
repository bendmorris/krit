.PHONY: all clean curl curses flac freetype harfbuzz jpeg libedit ogg openal openssl opus png sdl sdl_image sndfile sqlite vorbis yaml zip zlib
all: curl flac flatbuffers freetype harfbuzz jpeg ogg openal openssl opus png sdl sdl_image sndfile sqlite vorbis yaml zip zlib
clean:
	rm -rf lib/* `ls -d build/*/`

.MKDIR: $(shell mkdir -p build)

# curl
curl: lib/libcurl.a
build/curl-7.87.0.tar.xz:
	curl -L https://github.com/curl/curl/releases/download/curl-7_87_0/curl-7.87.0.tar.xz -o $@
build/curl-7.87.0: build/curl-7.87.0.tar.xz
	tar xf $< -C build
lib/libcurl.a: build/curl-7.87.0 lib/libssl.a
	cd $< && ./configure --build=$$BUILD --host=$$HOST --enable-static --disable-shared --disable-ldap --disable-ldaps --without-librtmp --with-openssl --prefix=$$PREFIX && make -j8 && make install

# curses
curses: lib/libncurses.a
build/ncurses-6.5.tar.gz:
	curl -L https://ftp.gnu.org/gnu/ncurses/ncurses-6.5.tar.gz -o $@
build/ncurses-6.5: build/ncurses-6.5.tar.gz
	tar xzf $< -C build
lib/libncurses.a: build/ncurses-6.5
	cd $< && ./configure --build=$$BUILD --host=$$HOST --enable-static --disable-shared --prefix=$$PREFIX 	--without-cxx \
    --without-ada \
    --with-cxx \
    --without-pthread \
    --disable-rpath \
    --enable-colorfgbg \
    --enable-ext-colors \
    --enable-ext-mouse \
    --disable-symlinks \
    --enable-warnings \
    --enable-assertions \
    --disable-home-terminfo \
    --enable-database \
    --enable-sp-funcs \
    --enable-term-driver \
    --enable-interop \
    --enable-widec \
    --enable-pc-files \
	--disable-lib-suffixes \
	--without-manpages && make -j8 && make install

# flac
flac: lib/libFLAC.a
build/flac-1.4.2.tar.xz:
	curl -L https://github.com/xiph/flac/releases/download/1.4.2/flac-1.4.2.tar.xz -o $@
build/flac-1.4.2: build/flac-1.4.2.tar.xz
	tar xf $< -C build
lib/libFLAC.a: build/flac-1.4.2 lib/libogg.a
	cd $< && ./configure --build=$$BUILD --host=$$HOST --enable-static --disable-shared --prefix=$$PREFIX --disable-stack-smash-protection && make -j8 && make install

# flatbuffers
FLATBUFFERS_VERSION=1.12.1
flatbuffers: lib/libflatbuffers.a
build/flatbuffers-$(FLATBUFFERS_VERSION).tar.gz:
	curl -L https://github.com/google/flatbuffers/archive/refs/tags/v$(FLATBUFFERS_VERSION).tar.gz -o $@
build/flatbuffers-$(FLATBUFFERS_VERSION): build/flatbuffers-$(FLATBUFFERS_VERSION).tar.gz
	tar xf $< -C build
lib/libflatbuffers.a: build/flatbuffers-$(FLATBUFFERS_VERSION)
	mkdir -p build/flatbuffers-$(FLATBUFFERS_VERSION)/build
	cd build/flatbuffers-$(FLATBUFFERS_VERSION)/build && cmake -DCMAKE_BUILD_TYPE=Release -G"Unix Makefiles" $$CMAKE_TOOLCHAIN -DCMAKE_INSTALL_PREFIX=$$PREFIX -D FLATBUFFERS_BUILD_TESTS=OFF -DCMAKE_POLICY_VERSION_MINIMUM=3.5 .. && make flatbuffers flatc -j8 && make install

# freetype
freetype: lib/libfreetype.a
build/freetype-2.12.1.tar.xz:
	curl -L https://download.savannah.gnu.org/releases/freetype/freetype-2.12.1.tar.xz -o $@
build/freetype-2.12.1: build/freetype-2.12.1.tar.xz
	tar xf $< -C build
lib/libfreetype.a: build/freetype-2.12.1 lib/libz.a
	cd $< && ./configure --build=$$BUILD --host=$$HOST --enable-static --disable-shared --prefix=$$PREFIX && make -j8 && make install

# harfbuzz
HARFBUZZ_VERSION=14.2.1
harfbuzz: lib/libharfbuzz.a
build/harfbuzz-${HARFBUZZ_VERSION}.tar.xz:
	curl -L https://github.com/harfbuzz/harfbuzz/releases/download/${HARFBUZZ_VERSION}/harfbuzz-${HARFBUZZ_VERSION}.tar.xz -o $@
build/harfbuzz-${HARFBUZZ_VERSION}: build/harfbuzz-${HARFBUZZ_VERSION}.tar.xz
	tar xf $< -C build
lib/libharfbuzz.a: build/harfbuzz-${HARFBUZZ_VERSION} lib/libfreetype.a
	cd $< && cmake -B build -DCMAKE_BUILD_TYPE=Release -G"Unix Makefiles" $$CMAKE_TOOLCHAIN -DCMAKE_INSTALL_PREFIX=$$PREFIX -DCMAKE_POSITION_INDEPENDENT_CODE=ON -D BUILD_SHARED_LIBS=OFF && cmake --build build -- -j8 libharfbuzz.a && cmake --build build -- install

# JPEG
jpeg: lib/libjpeg.a
build/libjpeg-turbo-3.1.4.1.tar.gz:
	curl -L https://github.com/libjpeg-turbo/libjpeg-turbo/releases/download/3.1.4.1/libjpeg-turbo-3.1.4.1.tar.gz -o $@
build/libjpeg-turbo-3.1.4.1: build/libjpeg-turbo-3.1.4.1.tar.gz
	tar xzf $< -C build
lib/libjpeg.a: build/libjpeg-turbo-3.1.4.1
	cd $< && cmake -B build -DCMAKE_BUILD_TYPE=Release -G"Unix Makefiles" $$CMAKE_TOOLCHAIN -DCMAKE_INSTALL_PREFIX=$$PREFIX -D ENABLE_SHARED=FALSE && cmake --build build -- jpeg-static -j8 && cmake --build build -- install

# libedit
libedit: lib/libedit.a
build/libedit-20240808-3.1.tar.gz:
	curl -L https://www.thrysoee.dk/editline/libedit-20240808-3.1.tar.gz -o $@
build/libedit-20240808-3.1: build/libedit-20240808-3.1.tar.gz
	tar xzf $< -C build
lib/libedit.a: build/libedit-20240808-3.1 lib/libncurses.a
	cd $< && ./configure --build=$$BUILD --host=$$HOST --enable-static --disable-shared --prefix=$$PREFIX && make -j8 && make install

# ogg
ogg: lib/libogg.a
build/libogg-1.3.5.tar.xz:
	curl -L https://github.com/xiph/ogg/releases/download/v1.3.5/libogg-1.3.5.tar.xz -o $@
build/libogg-1.3.5: build/libogg-1.3.5.tar.xz
	tar xf $< -C build
lib/libogg.a: build/libogg-1.3.5
	cd $< && ./configure --build=$$BUILD --host=$$HOST --enable-static --disable-shared --prefix=$$PREFIX && make -j8 && make install

# openal
openal: $(OPENAL_LIB_NAME)
build/openal-soft-1.24.0.tar.gz:
	curl -L https://github.com/kcat/openal-soft/archive/refs/tags/1.24.0.tar.gz -o $@
build/openal-soft-1.24.0: build/openal-soft-1.24.0.tar.gz
	tar xzf $< -C build
$(OPENAL_LIB_NAME): build/openal-soft-1.24.0 lib/libsndfile.a lib/libz.a
	cd $< && cmake -B build -DCMAKE_BUILD_TYPE=Release -G"Unix Makefiles" $$CMAKE_TOOLCHAIN -DCMAKE_INSTALL_PREFIX=$$PREFIX -DLIBTYPE=STATIC -DALSOFT_UTILS=OFF -DALSOFT_EXAMPLES=OFF -DSNDFILE_LIBRARY=$(shell pwd)/lib/libsndfile.a -DSNDFILE_INCLUDE_DIR=$(shell pwd)/include -DZLIB_LIBRARY=$(shell pwd)/lib/libz.a -DZLIB_INCLUDE_DIR=$(shell pwd)/include .. && cmake --build build -- -j8 && cmake --build build -- install

# openssl
openssl: lib/libssl.a
build/openssl-3.0.7.tar.gz:
	curl -L https://www.openssl.org/source/openssl-3.0.7.tar.gz -o $@
build/openssl-3.0.7: build/openssl-3.0.7.tar.gz
	tar xzf $< -C build
lib/libssl.a: build/openssl-3.0.7
	cd $< && ./Configure $$SSL_CONFIGURE --no-shared --static -static enable-capieng --prefix=$$PREFIX --libdir=lib && make -j8 && make install_sw
	touch $@

# opus
opus: lib/libopus.a
build/opus-1.1.2.tar.gz:
	curl -L https://github.com/xiph/opus/releases/download/v1.1.2/opus-1.1.2.tar.gz -o $@
build/opus-1.1.2: build/opus-1.1.2.tar.gz
	tar xzf $< -C build
lib/libopus.a: build/opus-1.1.2
	cd $< && ./configure --build=$$BUILD --host=$$HOST --enable-static --disable-shared --prefix=$$PREFIX && make -j8 && make install

# PNG
png: lib/libpng16.a
build/libpng-1.6.39.tar.gz:
	curl -L http://prdownloads.sourceforge.net/libpng/libpng-1.6.39.tar.gz?download -o $@
build/libpng-1.6.39: build/libpng-1.6.39.tar.gz
	tar xzf $< -C build
lib/libpng16.a: build/libpng-1.6.39 lib/libz.a
	cd $< && ./configure --build=$$BUILD --host=$$HOST --enable-static --disable-shared --prefix=$$PREFIX && make -j8 && make install

# SDL
SDL_VERSION=3.4.12
sdl: lib/libSDL3.a
build/SDL3-$(SDL_VERSION).tar.gz:
	mkdir -p build
	curl -L https://github.com/libsdl-org/SDL/releases/download/release-$(SDL_VERSION)/SDL3-$(SDL_VERSION).tar.gz -o $@
build/SDL3-$(SDL_VERSION): build/SDL3-$(SDL_VERSION).tar.gz
	tar xzf $< -C build
lib/libSDL3.a: build/SDL3-$(SDL_VERSION)
	cd $< && cmake -B build -DCMAKE_BUILD_TYPE=Release $$CMAKE_TOOLCHAIN -DSDL_SHARED=Off -DSDL_STATIC=On -DSDL_TEST_LIBRARY=Off -DSDL_AUDIO=Off -DSDL_VULKAN=Off -DSDL_LEAN_AND_MEAN=On -DCMAKE_INSTALL_PREFIX=$$PREFIX -DCMAKE_POSITION_INDEPENDENT_CODE=ON ${SDL_FLAGS} && cmake --build build -- -j8 && cmake --build build -- install

# SDL_image
SDL_IMAGE_VERSION=3.4.4
sdl_image: lib/libSDL3_image.a
build/SDL3_image-$(SDL_IMAGE_VERSION).tar.gz:
	curl -L https://github.com/libsdl-org/SDL_image/releases/download/release-$(SDL_IMAGE_VERSION)/SDL3_image-$(SDL_IMAGE_VERSION).tar.gz -o $@
build/SDL3_image-$(SDL_IMAGE_VERSION): build/SDL3_image-$(SDL_IMAGE_VERSION).tar.gz
	tar xzf $< -C build
lib/libSDL3_image.a: build/SDL3_image-$(SDL_IMAGE_VERSION) lib/libSDL3.a lib/libpng16.a lib/libjpeg.a
	cd $< && cmake -B build -DCMAKE_BUILD_TYPE=Release $$CMAKE_TOOLCHAIN -DBUILD_SHARED_LIBS=OFF -DSDLIMAGE_SAMPLES=OFF -DSDLIMAGE_DEPS_SHARED=OFF  -DSDLIMAGE_ANI=Off -DSDLIMAGE_AVIF=Off -DSDLIMAGE_BMP=Off -DSDLIMAGE_GIF=Off -DSDLIMAGE_JXL=Off -DSDLIMAGE_LBM=Off -DSDLIMAGE_PCX=Off -DSDLIMAGE_PNM=Off -DSDLIMAGE_QOI=Off -DSDLIMAGE_SVG=Off -DSDLIMAGE_TGA=Off -DSDLIMAGE_TIF=Off -DSDLIMAGE_WEBP=Off -DSDLIMAGE_XCF=Off -DSDLIMAGE_XPM=Off -DSDLIMAGE_XV=Off -DCMAKE_INSTALL_PREFIX=$$PREFIX && cmake --build build -- -j8 && cmake --build build -- install

# sndfile
sndfile: lib/libsndfile.a
build/libsndfile-1.2.2.tar.xz:
	curl -L https://github.com/libsndfile/libsndfile/releases/download/1.2.2/libsndfile-1.2.2.tar.xz -o $@
build/libsndfile-1.2.2: build/libsndfile-1.2.2.tar.xz
	tar xf $< -C build
lib/libsndfile.a: build/libsndfile-1.2.2 lib/libogg.a lib/libvorbis.a lib/libopus.a lib/libFLAC.a
	cd $< && CFLAGS="${CFLAGS} -std=c99" ./configure --build=$$BUILD --host=$$HOST --enable-static --disable-shared --prefix=$$PREFIX  && make -j8 && make install

# sqlite
sqlite: lib/libsqlite3.a
build/sqlite-autoconf-3400100.tar.gz:
	curl -L https://www.sqlite.org/2022/sqlite-autoconf-3400100.tar.gz -o $@
build/sqlite-autoconf-3400100: build/sqlite-autoconf-3400100.tar.gz
	tar xzf $< -C build
lib/libsqlite3.a: build/sqlite-autoconf-3400100
	cd $< && ./configure --build=$$BUILD --host=$$HOST --enable-static --disable-shared --prefix=$$PREFIX && make -j8 && make install

# vorbis
vorbis: lib/libvorbis.a
build/libvorbis-1.3.7.tar.xz:
	curl -L https://github.com/xiph/vorbis/releases/download/v1.3.7/libvorbis-1.3.7.tar.xz -o $@
build/libvorbis-1.3.7: build/libvorbis-1.3.7.tar.xz
	tar xf $< -C build
lib/libvorbis.a: build/libvorbis-1.3.7 lib/libogg.a
	cd $< && ./configure --build=$$BUILD --host=$$HOST --enable-static --disable-shared --prefix=$$PREFIX && make -j8 && make install

# yaml
yaml: lib/libyaml.a
build/yaml-0.2.5.tar.gz:
	curl -L https://github.com/yaml/libyaml/releases/download/0.2.5/yaml-0.2.5.tar.gz -o $@
build/yaml-0.2.5: build/yaml-0.2.5.tar.gz
	tar xzf $< -C build
lib/libyaml.a: build/yaml-0.2.5
	cd $< && ./configure --build=$$BUILD --host=$$HOST --enable-static --disable-shared --prefix=$$PREFIX && make -j8 && make install

# zip
zip: lib/libzip.a
build/libzip-1.11.4.tar.gz:
	curl -L https://github.com/nih-at/libzip/releases/download/v1.11.4/libzip-1.11.4.tar.gz -o $@
build/libzip-1.11.4: build/libzip-1.11.4.tar.gz
	tar xzf $< -C build
lib/libzip.a: build/libzip-1.11.4 lib/libz.a
	cd $< && mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Release $$CMAKE_TOOLCHAIN .. -DBUILD_SHARED_LIBS=Off -DENABLE_COMMONCRYPTO=Off -DENABLE_GNUTLS=Off -DENABLE_MBEDTLS=Off -DENABLE_OPENSSL=Off -DENABLE_WINDOWS_CRYPTO=Off -DENABLE_BZIP2=Off -DENABLE_LZMA=Off -DENABLE_ZSTD=Off -DBUILD_TOOLS=Off -DBUILD_REGRESS=Off -DBUILD_EXAMPLES=Off -DBUILD_DOC=Off -DCMAKE_FIND_ROOT_PATH=$$PREFIX -DCMAKE_INSTALL_PREFIX=$$PREFIX -DZLIB_LIBRARY=$(shell pwd)/lib/libz.a -DZLIB_INCLUDE_DIR=$(shell pwd)/include && make -j8 && make install

# ZLIB
zlib: lib/libz.a
build/zlib-1.3.2.tar.gz:
	curl -L https://zlib.net/zlib-1.3.2.tar.gz -o $@
build/zlib-1.3.2: build/zlib-1.3.2.tar.gz
	tar xzf $< -C build
lib/libz.a: build/zlib-1.3.2
	cd $< && ./configure --static --prefix=$$PREFIX && make -j8 && make install
