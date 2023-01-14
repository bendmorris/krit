#include "krit/Engine.h"
#include "krit/asset/AssetLoader.h"
#include "krit/asset/AssetType.h"
#include "krit/io/Io.h"
#include "krit/sound/MusicData.h"
#include "krit/sound/SoundData.h"
#include "krit/utils/Log.h"
#include <cstdint>
#include <memory.h>
#include <sndfile.h>

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

static SNDFILE *loadSoundFile(VirtualIo &io, const std::string &path,
                              SF_INFO &sfInfo, char **data) {
    *data = engine->io->read(path.c_str(), &io.length);
    io.data = *data;
    SNDFILE *sndFile = sf_open_virtual(&vio, SFM_READ, &sfInfo, &io);
    if (!sndFile) {
        Log::error("error loading sound asset %s: %s\n", path.c_str(),
                   sf_strerror(nullptr));
        engine->io->free(*data);
        *data = io.data = nullptr;
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
std::shared_ptr<SoundData>
AssetLoader<SoundData>::loadAsset(const std::string &path) {
    VirtualIo io;
    std::shared_ptr<SoundData> s = std::make_shared<SoundData>();
    SF_INFO sfInfo;
    char *fileData;
    SNDFILE *sndFile = loadSoundFile(io, path, sfInfo, &fileData);
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
    engine->io->free(fileData);

    alGenBuffers(1, &s->buffer);
    alBufferData(s->buffer, format, data, len * 2, s->sampleRate);
    delete[] data;

    return s;
}

template <> bool AssetLoader<SoundData>::assetIsReady(SoundData *s) {
    return s->buffer;
}

template <> size_t AssetLoader<SoundData>::cost(SoundData *s) {
    return s->channels * s->frames * 2;
}

template <> AssetType AssetLoader<SoundData>::type() { return SoundAsset; }

template <>
std::shared_ptr<MusicData>
AssetLoader<MusicData>::loadAsset(const std::string &path) {
    std::shared_ptr<MusicData> s = std::make_shared<MusicData>();
    SF_INFO sfInfo = {0};
    char *data = nullptr;
    SNDFILE *sndFile = loadSoundFile(s->io, path, sfInfo, &data);
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

template <> size_t AssetLoader<MusicData>::cost(MusicData *m) {
    return m->channels * m->frames * 2;
}

template <> AssetType AssetLoader<MusicData>::type() { return MusicAsset; }

template <> bool AssetLoader<MusicData>::assetIsReady(MusicData *s) {
    return s->sndFile;
}

}
