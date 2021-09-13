#include <cstdint>
#include <memory.h>
#include <sndfile.h>

#include "krit/asset/AssetInfo.h"
#include "krit/asset/AssetLoader.h"
#include "krit/io/Io.h"
#include "krit/sound/MusicData.h"
#include "krit/sound/SoundData.h"
#include "krit/utils/Log.h"

namespace krit {

static sf_count_t sf_vio_get_filelen(void *_io) {
    return static_cast<VirtualIo *>(_io)->length;
}
static sf_count_t sf_vio_seek(sf_count_t offset, int whence, void *_io) {
    VirtualIo *io = static_cast<VirtualIo *>(_io);
    switch (whence) {
        case SEEK_CUR: {
            io->cursor += offset;
            break;
        }
        case SEEK_SET: {
            io->cursor = offset;
            break;
        }
        case SEEK_END: {
            io->cursor = io->length + offset;
            break;
        }
    }
    return io->cursor;
}
static sf_count_t sf_vio_read(void *buf, sf_count_t bytes, void *_io) {
    VirtualIo *io = static_cast<VirtualIo *>(_io);
    int toRead = std::min(io->length - io->cursor, static_cast<int>(bytes));
    memcpy(buf, &io->data[io->cursor], toRead);
    io->cursor += toRead;
    return toRead;
}
static sf_count_t sf_vio_write(const void *buf, sf_count_t bytes, void *_io) {
    VirtualIo *io = static_cast<VirtualIo *>(_io);
    memcpy(&io->data[io->cursor], buf, bytes);
    io->cursor += bytes;
    return bytes;
}
static sf_count_t sf_vio_tell(void *_io) {
    return static_cast<VirtualIo *>(_io)->cursor;
}

static SF_VIRTUAL_IO vio = (SF_VIRTUAL_IO){
    .get_filelen = sf_vio_get_filelen,
    .seek = sf_vio_seek,
    .read = sf_vio_read,
    .write = sf_vio_write,
    .tell = sf_vio_tell,
};

static SNDFILE *loadSoundFile(VirtualIo &io, const AssetInfo &info,
                              SF_INFO &sfInfo, char **data) {
    *data = IoRead::read(info.path, &io.length);
    io.data = *data;
    SNDFILE *sndFile = sf_open_virtual(&vio, SFM_READ, &sfInfo, &io);
    if (!sndFile) {
        Log::error("error loading sound asset %s: %s\n", info.path.c_str(),
                   sf_strerror(nullptr));
        return nullptr;
    }
    return sndFile;
}

static ALenum soundFormat(SF_INFO &sfInfo) {
    if (sfInfo.channels == 1) {
        return AL_FORMAT_MONO16;
    } else if (sfInfo.channels == 2) {
        return AL_FORMAT_STEREO16;
    } else {
        Log::error("unsupported number of channels: %i", sfInfo.channels);
        return -1;
    }
}

template <>
SoundData *AssetLoader<SoundData>::loadAsset(const AssetInfo &info) {
    VirtualIo io;
    SoundData *s = new SoundData();
    SF_INFO sfInfo;
    char *fileData;
    SNDFILE *sndFile = loadSoundFile(io, info, sfInfo, &fileData);
    if (!sndFile) {
        return nullptr;
    }
    ALenum format = soundFormat(sfInfo);
    if (format == -1) {
        return nullptr;
    }

    s->sampleRate = sfInfo.samplerate;
    s->channels = sfInfo.channels;
    s->frames = sfInfo.frames;
    size_t len = s->channels * s->frames;
    int16_t *data = new int16_t[len];
    sf_read_short(sndFile, data, len);
    sf_close(sndFile);
    IoRead::free(fileData);

    alGenBuffers(1, &s->buffer);
    alBufferData(s->buffer, format, data, len * 2, s->sampleRate);

    return s;
}

template <> void AssetLoader<SoundData>::unloadAsset(SoundData *s) { delete s; }

template <> bool AssetLoader<SoundData>::assetIsReady(SoundData *s) {
    return s->buffer;
}

template <>
MusicData *AssetLoader<MusicData>::loadAsset(const AssetInfo &info) {
    MusicData *s = new MusicData();
    SF_INFO sfInfo;
    char *data;
    SNDFILE *sndFile = loadSoundFile(s->io, info, sfInfo, &data);
    if (!sndFile) {
        return nullptr;
    }

    s->format = soundFormat(sfInfo);
    s->sampleRate = sfInfo.samplerate;
    s->channels = sfInfo.channels;
    s->frames = sfInfo.frames;
    s->sndFile = sndFile;

    return s;
}

template <> void AssetLoader<MusicData>::unloadAsset(MusicData *s) { delete s; }

template <> bool AssetLoader<MusicData>::assetIsReady(MusicData *s) {
    return s->sndFile;
}

}