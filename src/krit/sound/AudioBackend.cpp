#include "krit/sound/AudioBackend.h"
#include "krit/App.h"
#include "krit/sound/MusicData.h"
#include "krit/sound/SoundData.h"
#include "krit/utils/Log.h"
#include <AL/alext.h>
#include <sndfile.h>

namespace krit {

static const int STREAM_BUFFER_SIZE = 64 * 1024;

AudioBackend::AudioBackend() {
    device = alcOpenDevice(nullptr);
    if (!device) {
        Log::error("couldn't open OpenAL default device");
        return;
    }

    context = alcCreateContext(device, nullptr);
    if (!context) {
        Log::error("couldn't create OpenAL context");
        return;
    }
    if (!alcMakeContextCurrent(context)) {
        Log::error("couldn't set OpenAL context");
        return;
    }

    ALuint __sources[MAX_SOURCES] = {0};
    alGenSources(MAX_SOURCES, __sources);
    sourcePool = &_sources[0];
    for (int i = 0; i < MAX_SOURCES; ++i) {
        _sources[i].source = __sources[i];
        if (i > 0) {
            _sources[i - 1].next = &_sources[i];
        }
    }

    for (int i = 0; i < MAX_STREAMS; ++i) {
        _streams[i] = AudioStream();
        _streams[i].backend = this;
        alGenBuffers(AudioStream::NUM_BUFFERS, _streams[i].buffer);
    }

    enabled = true;
}

AudioBackend::~AudioBackend() {
    ALuint __sources[MAX_SOURCES] = {0};
    for (int i = 0; i < MAX_SOURCES; ++i) {
        __sources[i] = _sources[i].source;
    }
    alDeleteSources(MAX_SOURCES, __sources);
    alcMakeContextCurrent(nullptr);
    if (context) {
        alcDestroyContext(context);
    }
    if (device) {
        alcCloseDevice(device);
    }
}

void AudioBackend::playSound(const std::string &name) {
    playSoundAsset(App::ctx.engine->getSound(name).get());
}

void AudioBackend::playSoundAsset(SoundData *sound) {
    if (enabled) {
        ALuint buffer = sound->buffer;
        AudioSource *source = getSource();
        if (source) {
            ALuint s = source->source;
            alSourcef(s, AL_PITCH, 1);
            alSourcef(s, AL_GAIN, soundVolume);
            alSource3f(s, AL_POSITION, 0, 0, 0);
            alSource3f(s, AL_VELOCITY, 0, 0, 0);
            alSourcei(s, AL_LOOPING, AL_FALSE);
            alSourcei(s, AL_BUFFER, buffer);
            alSourcePlay(s);
            source->next = activeSources;
            activeSources = source;
        }
    }
}

AudioSource *AudioBackend::getSource() {
    AudioSource *source = sourcePool;
    if (source) {
        sourcePool = source->next;
    } else {
        Log::warn("couldn't find an available audio source");
        return nullptr;
    }
    source->next = nullptr;
    Log::debug("get source: returning %i", (int)source->source);
    return source;
}

void AudioBackend::recycleSource(AudioSource *source) {
    source->next = sourcePool;
    sourcePool = source;
}

void AudioBackend::update() {
    // try to reclaim sources from active list
    AudioSource *candidate = activeSources;
    AudioSource *last = nullptr;
    // FIXME: with source reclamation, music crossfade sometimes breaks
    while (candidate) {
        ALenum state;
        alGetSourcei(candidate->source, AL_SOURCE_STATE, &state);
        if (state == AL_STOPPED) {
            Log::debug("reclaimed source: %i", (int)candidate->source);
            if (last) {
                last->next = candidate->next;
            } else {
                activeSources = candidate->next;
            }
            AudioSource *next = candidate->next;
            alSourcei(candidate->source, AL_BUFFER, 0);
            recycleSource(candidate);
            candidate = next;
        } else {
            last = candidate;
            candidate = candidate->next;
        }
    }

    // feed any audio streams that have completed a buffer
    for (int i = 0; i < MAX_STREAMS; ++i) {
        AudioStream &stream = _streams[i];
        if (stream.source) {
            ALint buffersProcessed = 0;
            alGetSourcei(stream.source->source, AL_BUFFERS_PROCESSED,
                         &buffersProcessed);
            while (buffersProcessed--) {
                stream.feed(false);
            }
        }
    }
}

AudioStream *AudioBackend::playMusic(const std::string &name) {
    return playMusicAsset(App::ctx.engine->getMusic(name));
}
AudioStream *AudioBackend::playMusicAsset(std::shared_ptr<MusicData> music) {
    int streamIndex = -1;
    for (int i = 0; i < MAX_STREAMS; ++i) {
        if (!_streams[i].data) {
            streamIndex = i;
            break;
        }
    }
    if (streamIndex < 0) {
        return nullptr;
    }
    AudioStream &stream = _streams[streamIndex];
    stream.clear();
    stream.data = music;
    if (!stream.ringBuffer) {
        stream.ringBuffer = std::unique_ptr<char[]>(
            new char[STREAM_BUFFER_SIZE * AudioStream::NUM_BUFFERS]);
    }
    stream.source = getSource();
    if (!stream.source) {
        // TODO: we should be more aggressive, prioritize taking an active
        // source for a new stream
        return nullptr;
    }
    alSourcef(stream.source->source, AL_PITCH, 1);
    alSourcef(stream.source->source, AL_GAIN, stream.volume);
    alSource3f(stream.source->source, AL_POSITION, 0, 0, 0);
    alSource3f(stream.source->source, AL_VELOCITY, 0, 0, 0);
    alSourcei(stream.source->source, AL_LOOPING, AL_FALSE);
    for (int i = 0; i < AudioStream::NUM_BUFFERS; ++i) {
        stream.feed(true);
    }
    alSourceQueueBuffers(stream.source->source, AudioStream::NUM_BUFFERS,
                         stream.buffer);
    return &stream;
}

void AudioStream::play() {
    if (source) {
        alSourcePlay(source->source);
    }
}

void AudioStream::pause() {
    if (source) {
        alSourcePause(source->source);
    }
}

void AudioStream::reset() {
    if (data) {
        sf_seek(data->sndFile, 0, SEEK_SET);
    }
}

void AudioStream::stop() {
    if (source) {
        alSourceStop(source->source);
        for (int i = 0; i < NUM_BUFFERS; ++i) {
            ALuint b;
            alSourceUnqueueBuffers(source->source, 1, &b);
        }
        backend->recycleSource(source);
        sf_seek(data->sndFile, 0, SEEK_SET);
        bufferPtr = 0;
        source = nullptr;
        data = nullptr;
    }
}

float AudioStream::setVolume(float v) {
    if (source) {
        alSourcef(source->source, AL_GAIN, volume = v);
        return v;
    } else {
        return 0;
    }
}

void AudioStream::feed(bool initial) {
    if (!data || !data->channels) {
        return;
    }
    int toRead = STREAM_BUFFER_SIZE / (data->channels * 4);
    memset(&ringBuffer[STREAM_BUFFER_SIZE * bufferPtr], 0, STREAM_BUFFER_SIZE);
    int read = sf_read_float(
        data->sndFile, (float *)(&ringBuffer[STREAM_BUFFER_SIZE * bufferPtr]),
        toRead);
    if (read < toRead) {
        if (onLoopData) {
            data = onLoopData;
            onLoopData = nullptr;
        } else {
            sf_seek(data->sndFile, 0, SEEK_SET);
        }
        // FIXME: slightly off; should only set samplesPlayed when this queued
        // buffer starts
        samplesPlayed = 0;
    }
    ALuint buffer;
    if (initial) {
        buffer = this->buffer[bufferPtr];
    } else {
        alSourceUnqueueBuffers(source->source, 1, &buffer);
        samplesPlayed += read / data->channels;
    }
    alBufferData(buffer, AL_FORMAT_STEREO_FLOAT32,
                 &ringBuffer[STREAM_BUFFER_SIZE * bufferPtr], read * 4,
                 data->sampleRate);
    if (!initial) {
        alSourceQueueBuffers(source->source, 1, &buffer);
    }
    ++bufferPtr;
    bufferPtr %= NUM_BUFFERS;
}

int AudioStream::sampleRate() { return data ? data->sampleRate : 0; }

float AudioStream::currentPlayTime() {
    int sampleOffset;
    alGetSourcei(source->source, AL_SAMPLE_OFFSET, &sampleOffset);
    return static_cast<float>(samplesPlayed + sampleOffset) / data->sampleRate;
}

// void AudioStream::onLoop(const std::string &name) {
//     onLoop(App::ctx.engine->getMusic(name));
// }
void AudioStream::onLoop(std::shared_ptr<MusicData> music) {
    onLoopData = music;
}

}
