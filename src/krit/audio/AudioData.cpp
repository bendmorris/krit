#include "krit/audio/AudioData.h"
#include "krit/Engine.h"
#include "krit/asset/AssetLoader.h"
#include "krit/io/Io.h"
#include "krit/utils/Log.h"
#include <cstdint>
#include <memory.h>
#include <sndfile.h>

namespace krit {

static sf_count_t sf_vio_get_filelen(void *_io) {
    return static_cast<AudioDataCursor *>(_io)->size;
}

static sf_count_t sf_vio_seek(sf_count_t offset, int whence, void *_io) {
    AudioDataCursor *io = static_cast<AudioDataCursor *>(_io);
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
            io->cursor = io->size + offset;
            break;
        }
    }
    return io->cursor;
}

static sf_count_t sf_vio_read(void *buf, sf_count_t bytes, void *_io) {
    AudioDataCursor *io = static_cast<AudioDataCursor *>(_io);
    int toRead =
        std::min<int>(static_cast<int>(io->audioData->data.size()) - io->cursor,
                      static_cast<int>(bytes));
    memcpy(buf, &io->audioData->data.c_str()[io->cursor], toRead);
    io->cursor += toRead;
    return toRead;
}

static sf_count_t sf_vio_write(const void *buf, sf_count_t bytes, void *_io) {
    AudioDataCursor *io = static_cast<AudioDataCursor *>(_io);
    memcpy((void *)&io->audioData->data.c_str()[io->cursor], buf, bytes);
    io->cursor += bytes;
    return bytes;
}

static sf_count_t sf_vio_tell(void *_io) {
    return static_cast<AudioDataCursor *>(_io)->cursor;
}

static SF_VIRTUAL_IO vio = (SF_VIRTUAL_IO){
    .get_filelen = sf_vio_get_filelen,
    .seek = sf_vio_seek,
    .read = sf_vio_read,
    .write = sf_vio_write,
    .tell = sf_vio_tell,
};

static ALenum soundFormat(SF_INFO &sfInfo) {
    if (sfInfo.channels == 1) {
        return AL_FORMAT_MONO16;
    } else if (sfInfo.channels == 2) {
        return AL_FORMAT_STEREO16;
    } else {
        LOG_ERROR("unsupported number of channels: %i", sfInfo.channels);
        return -1;
    }
}

AudioDataCursor::AudioDataCursor(std::shared_ptr<AudioData> data)
    : audioData(data), size(data->data.size()), sndFile(nullptr, sf_close) {
    SF_INFO sfInfo{0};
    SNDFILE *sndFile = sf_open_virtual(&vio, SFM_READ, &sfInfo, (void *)(this));
    if (!sndFile) {
        AREA_LOG_ERROR("audio", "error loading sound asset %s: %s",
                       data->path.c_str(), sf_strerror(nullptr));
    }
    if (!data->format) {
        data->format = soundFormat(sfInfo);
        data->sampleRate = sfInfo.samplerate;
        data->channels = sfInfo.channels;
        data->frames = sfInfo.frames;
    }
    this->sndFile = decltype(this->sndFile)(sndFile, sf_close);
}

template <>
std::shared_ptr<AudioData>
AssetLoader<AudioData>::loadAsset(const std::string &path) {
    AREA_LOG_INFO("audio", "loading audio asset %s", path.c_str());
    std::shared_ptr<AudioData> a = std::make_shared<AudioData>();
    std::string content = engine->io->readFile(path.c_str());
    a->path = path;
    a->data = std::move(content);
    return a;
}

template <> size_t AssetLoader<AudioData>::cost(AudioData *a) {
    return a->data.size();
}

template <> AssetType AssetLoader<AudioData>::type() { return AudioAsset; }

template <> bool AssetLoader<AudioData>::assetIsReady(AudioData *a) {
    return !!a;
}

}
