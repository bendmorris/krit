.PHONY: all clean curl curses flac freetype harfbuzz jpeg libedit ogg openal openssl opus png sndfile sqlite vorbis yaml zip zlib
all: curl flac flatbuffers freetype harfbuzz jpeg ogg openal openssl opus png sndfile sqlite vorbis yaml zip zlib
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
flatbuffers: lib/libflatbuffers.a
build/flatbuffers.tar.gz:
	curl -L https://github.com/google/flatbuffers/archive/refs/tags/v25.12.19-2026-02-06-03fffb2.tar.gz -o $@
build/flatbuffers-25.12.19-2026-02-06-03fffb2: build/flatbuffers.tar.gz
	tar xf $< -C build
lib/libflatbuffers.a: build/flatbuffers-25.12.19-2026-02-06-03fffb2
	mkdir -p build/flatbuffers-25.12.19-2026-02-06-03fffb2/build
	cd build/flatbuffers-25.12.19-2026-02-06-03fffb2/build && cmake -G"Unix Makefiles" $$CMAKE_TOOLCHAIN -DCMAKE_INSTALL_PREFIX=$$PREFIX -D ENABLE_SHARED=FALSE .. && make flatbuffers -j8 && make install

# freetype
freetype: lib/libfreetype.a
build/freetype-2.12.1.tar.xz:
	curl -L https://download.savannah.gnu.org/releases/freetype/freetype-2.12.1.tar.xz -o $@
build/freetype-2.12.1: build/freetype-2.12.1.tar.xz
	tar xf $< -C build
lib/libfreetype.a: build/freetype-2.12.1 lib/libz.a
	cd $< && ./configure --build=$$BUILD --host=$$HOST --enable-static --disable-shared --prefix=$$PREFIX && make -j8 && make install

# harfbuzz
harfbuzz: lib/libharfbuzz.a
build/harfbuzz-6.0.0.tar.xz:
	curl -L https://github.com/harfbuzz/harfbuzz/releases/download/6.0.0/harfbuzz-6.0.0.tar.xz -o $@
build/harfbuzz-6.0.0: build/harfbuzz-6.0.0.tar.xz
	tar xf $< -C build
lib/libharfbuzz.a: build/harfbuzz-6.0.0 lib/libfreetype.a
	cd $< && meson build $$MESON_CROSS -Dprefix=$$PREFIX -Ddefault_library=static -Dtests=disabled -Dc_args="-O3 $$MESON_CFLAGS" -Dcpp_args="-O3 $$MESON_CFLAGS" -Dc_link_args="-static $$MESON_LDFLAGS" -Dcpp_link_args="-static $$MESON_LDFLAGS" && meson compile -C build && ninja src/libharfbuzz.a -C build && meson install -C build && cp build/src/libharfbuzz.a ../../lib/

# JPEG
jpeg: lib/libjpeg.a
build/libjpeg-turbo-3.1.4.1.tar.gz:
	curl -L https://github.com/libjpeg-turbo/libjpeg-turbo/releases/download/3.1.4.1/libjpeg-turbo-3.1.4.1.tar.gz -o $@
build/libjpeg-turbo-3.1.4.1: build/libjpeg-turbo-3.1.4.1.tar.gz
	tar xzf $< -C build
lib/libjpeg.a: build/libjpeg-turbo-3.1.4.1
	mkdir -p build/libjpeg-turbo-3.1.4.1/build
	cd build/libjpeg-turbo-3.1.4.1/build && cmake -G"Unix Makefiles" $$CMAKE_TOOLCHAIN -DCMAKE_INSTALL_PREFIX=$$PREFIX -D ENABLE_SHARED=FALSE .. && make jpeg-static -j8 && make install

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
	cd build/openal-soft-1.24.0/build && cmake -G"Unix Makefiles" $$CMAKE_TOOLCHAIN -DCMAKE_INSTALL_PREFIX=$$PREFIX -DLIBTYPE=STATIC -DALSOFT_UTILS=OFF -DALSOFT_EXAMPLES=OFF -DSNDFILE_LIBRARY=$(shell pwd)/lib/libsndfile.a -DSNDFILE_INCLUDE_DIR=$(shell pwd)/include -DZLIB_LIBRARY=$(shell pwd)/lib/libz.a -DZLIB_INCLUDE_DIR=$(shell pwd)/include .. && env && make -j8 && make install

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
# sdl: lib/libSDL2.a
# build/SDL2-2.32.10.tar.gz:
# 	mkdir -p build
# 	curl -L https://github.com/libsdl-org/SDL/releases/download/release-2.32.10/SDL2-2.32.10.tar.gz -o $@
# build/SDL2-2.32.10: build/SDL2-2.32.10.tar.gz
# 	tar xzf $< -C build
# lib/libSDL2.a: build/SDL2-2.32.10
# 	cd $< && cmake -B build -G"Unix Makefiles" $$CMAKE_TOOLCHAIN -DCMAKE_INSTALL_PREFIX=$$PREFIX -D ENABLE_SHARED=FALSE
# 	cd $< && cmake --build build && cmake --build build --target install

# SDL_image
# sdl_image: lib/libSDL2_image.a
# build/SDL2_image-2.8.12.tar.gz:
# 	curl -L https://github.com/libsdl-org/SDL_image/releases/download/release-2.8.12/SDL2_image-2.8.12.tar.gz -o $@
# build/SDL2_image-2.8.12: build/SDL2_image-2.8.12.tar.gz
# 	tar xzf $< -C build
# lib/libSDL2_image.a: build/SDL2_image-2.8.12 lib/libpng16.a lib/libjpeg.a
# 	cd $< && ./configure --build=$$BUILD --host=$$HOST --enable-static --disable-shared --disable-png-shared --disable-jpg-shared --prefix=$$PREFIX --with-sdl-prefix=$$PREFIX && make -j8 && make install

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
	cd $< && mkdir -p build && cd build && cmake $$CMAKE_TOOLCHAIN .. -DBUILD_SHARED_LIBS=Off -DENABLE_COMMONCRYPTO=Off -DENABLE_GNUTLS=Off -DENABLE_MBEDTLS=Off -DENABLE_OPENSSL=Off -DENABLE_WINDOWS_CRYPTO=Off -DENABLE_BZIP2=Off -DENABLE_LZMA=Off -DENABLE_ZSTD=Off -DBUILD_TOOLS=Off -DBUILD_REGRESS=Off -DBUILD_EXAMPLES=Off -DBUILD_DOC=Off -DCMAKE_FIND_ROOT_PATH=$$PREFIX -DCMAKE_INSTALL_PREFIX=$$PREFIX -DZLIB_LIBRARY=$(shell pwd)/lib/libz.a -DZLIB_INCLUDE_DIR=$(shell pwd)/include && make -j8 && make install

# ZLIB
zlib: lib/libz.a
build/zlib-1.3.2.tar.gz:
	curl -L https://zlib.net/zlib-1.3.2.tar.gz -o $@
build/zlib-1.3.2: build/zlib-1.3.2.tar.gz
	tar xzf $< -C build
lib/libz.a: build/zlib-1.3.2
	cd $< && ./configure --static --prefix=$$PREFIX && make -j8 && make install
