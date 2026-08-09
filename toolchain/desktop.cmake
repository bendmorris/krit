include(ExternalProject)

set(KRIT_BACKEND_PLATFORM Desktop CACHE STRING "Platform backend")
set(KRIT_BACKEND_RENDERER Gl CACHE STRING "Renderer backend")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    set(AUTOTOOLS_BUILD_TRIPLET "x86_64-pc-linux-gnu")
else()
    message(FATAL "unrecognized build system")
endif()

option(KRIT_ENABLE_DIALOGS "Enable system file dialogs" ON)
if(KRIT_ENABLE_DIALOGS)
    add_compile_definitions(krit INTERFACE -DKRIT_ENABLE_DIALOGS)
    include_directories(${CMAKE_CURRENT_LIST_DIR}/../tinyfiledialogs)
    list(APPEND KRIT_SRC_FILES ${CMAKE_CURRENT_LIST_DIR}/../tinyfiledialogs/tinyfiledialogs.c)
endif()

target_compile_definitions(krit INTERFACE -DKRIT_DESKTOP)
if(KRIT_ENABLE_THREADS)
    target_compile_definitions(krit INTERFACE -DKRIT_SOUND_THREAD)
endif()
target_include_directories(krit INTERFACE ${CMAKE_CURRENT_LIST_DIR}/../argparse/include)

# dynamic libs
find_package(OpenGL REQUIRED)
list(APPEND KRIT_LIBS
    ${OPENGL_LIBRARIES}
)

set(TOOLCHAIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/toolchain")
set(AUTOTOOLS_COMMON "--build=${AUTOTOOLS_BUILD_TRIPLET};--host=${AUTOTOOLS_HOST_TRIPLET};--prefix=${TOOLCHAIN_DIR}")
set(CMAKE_COMMON "-DCMAKE_INSTALL_PREFIX=${TOOLCHAIN_DIR}")
target_link_directories(krit INTERFACE ${TOOLCHAIN_DIR}/lib)
target_include_directories(krit INTERFACE ${TOOLCHAIN_DIR}/include)

ExternalProject_Add(jpeg
    URL https://github.com/libjpeg-turbo/libjpeg-turbo/releases/download/3.1.4.1/libjpeg-turbo-3.1.4.1.tar.gz
    CMAKE_ARGS ${CMAKE_COMMON} -D ENABLE_SHARED=FALSE
)
ExternalProject_Add(png
    URL http://prdownloads.sourceforge.net/libpng/libpng-1.6.39.tar.gz?download
    CONFIGURE_COMMAND <SOURCE_DIR>/configure ${AUTOTOOLS_COMMON} --enable-static --disable-shared
    BUILD_COMMAND $(MAKE)
    INSTALL_COMMAND $(MAKE) install
    USES_TERMINAL_CONFIGURE TRUE
)
ExternalProject_Add(yaml
    URL https://github.com/yaml/libyaml/releases/download/0.2.5/yaml-0.2.5.tar.gz
    CONFIGURE_COMMAND <SOURCE_DIR>/configure ${AUTOTOOLS_COMMON} --enable-static --disable-shared
    BUILD_COMMAND $(MAKE)
    INSTALL_COMMAND $(MAKE) install
    USES_TERMINAL_CONFIGURE TRUE
)
ExternalProject_Add(z
    URL https://zlib.net/zlib-1.3.2.tar.gz
    CONFIGURE_COMMAND <SOURCE_DIR>/configure ${AUTOTOOLS_COMMON} --static
    BUILD_COMMAND $(MAKE)
    INSTALL_COMMAND $(MAKE) install
    USES_TERMINAL_CONFIGURE TRUE
)
ExternalProject_Add(zip
    URL https://github.com/nih-at/libzip/releases/download/v1.11.4/libzip-1.11.4.tar.gz
    CMAKE_ARGS -DBUILD_SHARED_LIBS=Off -DENABLE_COMMONCRYPTO=Off -DENABLE_GNUTLS=Off -DENABLE_MBEDTLS=Off -DENABLE_OPENSSL=Off -DENABLE_WINDOWS_CRYPTO=Off -DENABLE_BZIP2=Off -DENABLE_LZMA=Off -DENABLE_ZSTD=Off -DBUILD_TOOLS=Off -DBUILD_REGRESS=Off -DBUILD_EXAMPLES=Off -DBUILD_DOC=Off -DCMAKE_INSTALL_PREFIX=${TOOLCHAIN_DIR}
)
ExternalProject_Add(sdl
    URL https://github.com/libsdl-org/SDL/releases/download/release-3.4.12/SDL3-3.4.12.tar.gz
    CMAKE_ARGS ${CMAKE_COMMON} -DSDL_SHARED=Off -DSDL_STATIC=On -DSDL_TEST_LIBRARY=Off -DSDL_AUDIO=Off -DSDL_RENDER=Off -DSDL_CAMERA=Off -DSDL_HAPTIC=Off -DSDL_POWER=Off -DSDL_SENSOR=Off -DSDL_DIALOG=Off -DSDL_TRAY=Off -DSDL_VULKAN=Off -DSDL_LEAN_AND_MEAN=On -DSDL_X11_XTEST=Off ${SDL_CMAKE_ARGS}
)
ExternalProject_Add(sdl_image
    URL https://github.com/libsdl-org/SDL_image/releases/download/release-3.4.4/SDL3_image-3.4.4.tar.gz
    CMAKE_ARGS ${CMAKE_COMMON} -DBUILD_SHARED_LIBS=OFF -DSDLIMAGE_SAMPLES=OFF -DSDLIMAGE_DEPS_SHARED=OFF  -DSDLIMAGE_ANI=Off -DSDLIMAGE_AVIF=Off -DSDLIMAGE_BMP=Off -DSDLIMAGE_GIF=Off -DSDLIMAGE_JXL=Off -DSDLIMAGE_LBM=Off -DSDLIMAGE_PCX=Off -DSDLIMAGE_PNM=Off -DSDLIMAGE_QOI=Off -DSDLIMAGE_SVG=Off -DSDLIMAGE_TGA=Off -DSDLIMAGE_TIF=Off -DSDLIMAGE_WEBP=Off -DSDLIMAGE_XCF=Off -DSDLIMAGE_XPM=Off -DSDLIMAGE_XV=Off
)
add_dependencies(krit jpeg png yaml z zip sdl sdl_image)

list(APPEND KRIT_STATIC_LIBS
    SDL3
    SDL3_image
    jpeg
    png
    yaml
    zip
    z
)

if(KRIT_ENABLE_NET)
    ExternalProject_Add(openssl
        URL https://www.openssl.org/source/openssl-3.0.7.tar.gz
        CONFIGURE_COMMAND <SOURCE_DIR>/Configure --prefix=${TOOLCHAIN_DIR} --static -static enable-capieng --libdir=lib
        BUILD_COMMAND $(MAKE)
        INSTALL_COMMAND $(MAKE) install_sw
        USES_TERMINAL_CONFIGURE TRUE
    )
    ExternalProject_Add(curl
        URL https://github.com/curl/curl/releases/download/curl-7_87_0/curl-7.87.0.tar.xz
        CONFIGURE_COMMAND <SOURCE_DIR>/configure ${AUTOTOOLS_COMMON} --enable-static --disable-shared --disable-ldap --disable-ldaps --without-librtmp --with-openssl=${TOOLCHAIN_DIR} --without-brotli --without-zstd
        BUILD_COMMAND $(MAKE)
        INSTALL_COMMAND $(MAKE) install
        USES_TERMINAL_CONFIGURE TRUE
        DEPENDS openssl
    )
    set(KRIT_BACKEND_NET Curl CACHE STRING "Net backend")
    target_compile_definitions(krit INTERFACE -DCURL_STATICLIB)
    list(APPEND KRIT_STATIC_LIBS
        curl
        ssl
        crypto
    )
    add_dependencies(krit openssl curl)
endif()
if(KRIT_ENABLE_TEXT)
    ExternalProject_Add(freetype
        URL https://download.savannah.gnu.org/releases/freetype/freetype-2.12.1.tar.xz
        CONFIGURE_COMMAND <SOURCE_DIR>/configure ${AUTOTOOLS_COMMON} --enable-static --disable-shared --without-bzip2 --without-brotli --without-harfbuzz
        BUILD_COMMAND $(MAKE)
        INSTALL_COMMAND $(MAKE) install
        USES_TERMINAL_CONFIGURE TRUE
    )
    ExternalProject_Add(harfbuzz
        URL https://github.com/harfbuzz/harfbuzz/releases/download/14.2.1/harfbuzz-14.2.1.tar.xz
        CMAKE_ARGS ${CMAKE_COMMON} -D BUILD_SHARED_LIBS=OFF -D HB_HAVE_FREETYPE=Off
    )
    list(APPEND KRIT_STATIC_LIBS
        harfbuzz
        freetype
    )
    target_include_directories(krit INTERFACE ${TOOLCHAIN_DIR}/include/freetype2)
    add_dependencies(krit freetype harfbuzz)
endif()
if(KRIT_ENABLE_AUDIO)
    ExternalProject_Add(flac
        URL https://github.com/xiph/flac/releases/download/1.4.2/flac-1.4.2.tar.xz
        CONFIGURE_COMMAND <SOURCE_DIR>/configure ${AUTOTOOLS_COMMON} --enable-static --disable-shared --disable-stack-smash-protection
        BUILD_COMMAND $(MAKE)
        INSTALL_COMMAND $(MAKE) install
        USES_TERMINAL_CONFIGURE TRUE
    )
    ExternalProject_Add(ogg
        URL https://github.com/xiph/ogg/releases/download/v1.3.5/libogg-1.3.5.tar.xz
        CONFIGURE_COMMAND <SOURCE_DIR>/configure ${AUTOTOOLS_COMMON} --enable-static --disable-shared
        BUILD_COMMAND $(MAKE)
        INSTALL_COMMAND $(MAKE) install
        USES_TERMINAL_CONFIGURE TRUE
    )
    ExternalProject_Add(vorbis
        URL https://github.com/xiph/vorbis/releases/download/v1.3.7/libvorbis-1.3.7.tar.xz
        CONFIGURE_COMMAND <SOURCE_DIR>/configure ${AUTOTOOLS_COMMON} --enable-static --disable-shared
        BUILD_COMMAND $(MAKE)
        INSTALL_COMMAND $(MAKE) install
        USES_TERMINAL_CONFIGURE TRUE
    )
    ExternalProject_Add(opus
        URL https://github.com/xiph/opus/releases/download/v1.1.2/opus-1.1.2.tar.gz
        CONFIGURE_COMMAND <SOURCE_DIR>/configure ${AUTOTOOLS_COMMON} --enable-static --disable-shared
        BUILD_COMMAND $(MAKE)
        INSTALL_COMMAND $(MAKE) install
        USES_TERMINAL_CONFIGURE TRUE
    )
    ExternalProject_Add(sndfile
        URL https://github.com/libsndfile/libsndfile/releases/download/1.2.2/libsndfile-1.2.2.tar.xz
        CONFIGURE_COMMAND <SOURCE_DIR>/configure ${AUTOTOOLS_COMMON} --enable-static --disable-shared --disable-stack-smash-protection
        CONFIGURE_ENVIRONMENT_MODIFICATION "CFLAGS=string_append:-std=c99"
        BUILD_COMMAND $(MAKE)
        INSTALL_COMMAND $(MAKE) install
        USES_TERMINAL_CONFIGURE TRUE
        DEPENDS flac ogg vorbis opus
    )
    ExternalProject_Add(openal
        URL https://github.com/kcat/openal-soft/archive/refs/tags/1.24.0.tar.gz
        CMAKE_ARGS ${CMAKE_COMMON} -DLIBTYPE=STATIC -DALSOFT_UTILS=OFF -DALSOFT_EXAMPLES=OFF -DSNDFILE_LIBRARY=${TOOLCHAIN_DIR}/lib/libsndfile.a -DSNDFILE_INCLUDE_DIR=${TOOLCHAIN_DIR}/include -DZLIB_LIBRARY=${TOOLCHAIN_DIR}/lib/libz.a -DZLIB_INCLUDE_DIR=${TOOLCHAIN_DIR}/include
        DEPENDS sndfile z
    )
    target_compile_definitions(krit INTERFACE -DAL_LIBTYPE_STATIC -DAL_ALEXT_PROTOTYPES)
    list(APPEND KRIT_STATIC_LIBS
        openal
        sndfile
        vorbis
        vorbisenc
        FLAC
        opus
        ogg
    )
    add_dependencies(krit flac ogg vorbis opus sndfile openal)
endif()
if(KRIT_ENABLE_SQLITE)
    ExternalProject_Add(sqlite3
        URL https://www.sqlite.org/2022/sqlite-autoconf-3400100.tar.gz
        CONFIGURE_COMMAND <SOURCE_DIR>/configure ${AUTOTOOLS_COMMON} --enable-static --disable-shared
        BUILD_COMMAND $(MAKE)
        INSTALL_COMMAND $(MAKE) install
        USES_TERMINAL_CONFIGURE TRUE
    )
    list(APPEND KRIT_STATIC_LIBS
        sqlite3
    )
    add_dependencies(krit sqlite3)
endif()
