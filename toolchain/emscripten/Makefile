export PREFIX=$(shell pwd)

.PHONY: vorbis opus flac sndfile

# ogg
ogg: lib/libogg.a
libogg-1.3.5.tar.xz:
	curl -L https://github.com/xiph/ogg/releases/download/v1.3.5/libogg-1.3.5.tar.xz -o $@
libogg-1.3.5: libogg-1.3.5.tar.xz
	tar xf $<
lib/libogg.a: libogg-1.3.5
	cd $< && emconfigure ./configure --disable-shared --prefix=$$PREFIX && emmake make -j8 && emmake make install

# vorbis
vorbis: lib/libvorbis.a
libvorbis-1.3.7.tar.xz:
	curl -L https://github.com/xiph/vorbis/releases/download/v1.3.7/libvorbis-1.3.7.tar.xz -o $@
libvorbis-1.3.7: libvorbis-1.3.7.tar.xz
	tar xf $<
lib/libvorbis.a: libvorbis-1.3.7
	cd $< && emconfigure ./configure --disable-shared --prefix=$$PREFIX && emmake make -j8 && emmake make install

# opus
opus: lib/libopus.a
opus-1.1.2.tar.gz:
	curl -L https://github.com/xiph/opus/releases/download/v1.1.2/opus-1.1.2.tar.gz -o $@
opus-1.1.2: opus-1.1.2.tar.gz
	tar xzf $<
lib/libopus.a: opus-1.1.2
	cd $< && emconfigure ./configure --disable-shared --prefix=$$PREFIX && emmake make -j8 && emmake make install

# flac
flac: lib/libflac.a
flac-1.4.2.tar.xz:
	curl -L https://github.com/xiph/flac/releases/download/1.4.2/flac-1.4.2.tar.xz -o $@
flac-1.4.2: flac-1.4.2.tar.xz
	tar xf $<
lib/libflac.a: flac-1.4.2
	cd $< && emconfigure ./configure --disable-shared --prefix=$$PREFIX && emmake make -j8 && emmake make install

# sndfile
sndfile: lib/libsndfile.a
libsndfile-1.2.0.tar.xz:
	curl -L https://github.com/libsndfile/libsndfile/releases/download/1.2.0/libsndfile-1.2.0.tar.xz -o $@
libsndfile-1.2.0: libsndfile-1.2.0.tar.xz
	tar xf $<
lib/libsndfile.a: libsndfile-1.2.0
	cd $< && emconfigure ./configure --enable-static --disable-shared --prefix=$$PREFIX --enable-external-libs PKG_CONFIG_PATH=$(shell pwd)/lib/pkgconfig && emmake make -j8 && emmake make install
