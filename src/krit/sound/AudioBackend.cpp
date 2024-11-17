#include "krit/sound/AudioBackend.h"
#include "krit/Engine.h"
#include "krit/sound/MusicData.h"
#include "krit/sound/SoundData.h"
#include "krit/utils/Log.h"
#include <AL/alext.h>
#include <sndfile.h>

namespace krit {

#if KRIT_RELEASE
#define checkForAlErrors(...)
#else
static void _checkForAlErrors(const char *fmt, ...) {
    ALenum err = alGetError();
    if (err != AL_NO_ERROR) {
        va_list args;
        va_start(args, fmt);
        fprintf(stderr, "AL error: %i ", err);
        vfprintf(stderr, fmt, args);
        fputs("\n", stderr);
        vpanic(fmt, args);
    }
}

#define STRINGIZE_DETAIL(x) #x
#define STRINGIZE(x) STRINGIZE_DETAIL(x)
#define checkForAlErrors(...)                                                  \
    _checkForAlErrors("AL Error at " __FILE__                                  \
                      ":" STRINGIZE(__LINE__) ": " __VA_ARGS__)
void _checkForAlErrors(const char *fmt, ...);
#endif

static const int STREAM_BUFFER_SIZE = 32 * 1024;

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
    checkForAlErrors("initial");

    streamFloats = alIsExtensionPresent("AL_EXT_float32");
    bytesPerSample = streamFloats ? 4 : 2;

    ALuint __sources[MAX_SOURCES] = {0};
    alGenSources(MAX_SOURCES, __sources);
    checkForAlErrors("alGenSources");
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
        alGenBuffers(AudioStream::NUM_BUFFERS, &_streams[i].buffer[0]);
        checkForAlErrors("alGenBuffers");
    }

    enabled = true;
}

AudioBackend::~AudioBackend() {
    ALuint __sources[MAX_SOURCES] = {0};
    for (int i = 0; i < MAX_SOURCES; ++i) {
        __sources[i] = _sources[i].source;
    }
    alDeleteSources(MAX_SOURCES, __sources);
    for (int i = 0; i < MAX_STREAMS; ++i) {
        alDeleteBuffers(AudioStream::NUM_BUFFERS, &_streams[i].buffer[0]);
    }
    alcMakeContextCurrent(nullptr);
    if (context) {
        alcDestroyContext(context);
    }
    if (device) {
        alcCloseDevice(device);
    }
}

void AudioBackend::playSound(const std::string &name) {
    auto sound = engine->getSound(name);
    if (sound) {
        playSoundAsset(sound.get());
    }
}

void AudioBackend::playSoundAsset(SoundData *sound) {
    if (enabled && sound) {
        ALuint buffer = sound->buffer;
        if (buffer) {
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
                checkForAlErrors("alSourcePlay");
                source->next = activeSources;
                activeSources = source;
            }
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
        checkForAlErrors("alGetSourcei");
        if (state == AL_STOPPED) {
            Log::debug("reclaimed source: %i", (int)candidate->source);
            if (last) {
                last->next = candidate->next;
            } else {
                activeSources = candidate->next;
            }
            AudioSource *next = candidate->next;
            alSourcei(candidate->source, AL_BUFFER, 0);
            checkForAlErrors("alSourcei");
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
            checkForAlErrors("alGetSourcei");
            bool needBuffers = buffersProcessed > 0;
            while (buffersProcessed--) {
                stream.feed(false);
            }
            if (needBuffers && stream.playing) {
                if (stream.playing) {
                    ALenum state;
                    alGetSourcei(stream.source->source, AL_SOURCE_STATE,
                                 &state);
                    if (state != AL_PLAYING) {
                        alSourcePlay(stream.source->source);
                    }
                }
            }
        }
    }
}

AudioStream *AudioBackend::playMusic(const std::string &name) {
    return playMusicAsset(engine->getMusic(name));
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
        Log::error("couldn't get an audio stream");
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
        Log::error("couldn't get an audio source");
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
                         &stream.buffer[0]);
    checkForAlErrors("alSourceQueueBuffers");
    return &stream;
}

void AudioStream::play() {
    if (source) {
        alSourcePlay(source->source);
        checkForAlErrors("alSourcePlay");
        playing = true;
    }
}

void AudioStream::pause() {
    if (source) {
        alSourcePause(source->source);
        checkForAlErrors("alSourcePause");
        playing = false;
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
        checkForAlErrors("alSourceStop");
        for (int i = 0; i < NUM_BUFFERS; ++i) {
            ALuint b;
            alSourceUnqueueBuffers(source->source, 1, &b);
            checkForAlErrors("alSourceUnqueueBuffers");
        }
        backend->recycleSource(source);
        if (data) {
            sf_seek(data->sndFile, 0, SEEK_SET);
        }
        bufferPtr = 0;
        source = nullptr;
        data = nullptr;
        playing = false;
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
    int toRead =
        STREAM_BUFFER_SIZE / (data->channels * backend->bytesPerSample);
    memset(&ringBuffer[STREAM_BUFFER_SIZE * bufferPtr], 0, STREAM_BUFFER_SIZE);
    int read =
        backend->streamFloats
            ? sf_read_float(
                  data->sndFile,
                  (float *)(&ringBuffer[STREAM_BUFFER_SIZE * bufferPtr]),
                  toRead)
            : sf_read_short(
                  data->sndFile,
                  (int16_t *)(&ringBuffer[STREAM_BUFFER_SIZE * bufferPtr]),
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
        checkForAlErrors("alSourceUnqueueBuffers");
        samplesPlayed += read / data->channels;
    }
    alBufferData(buffer,
                 backend->streamFloats ? AL_FORMAT_STEREO_FLOAT32
                                       : AL_FORMAT_STEREO16,
                 &ringBuffer[STREAM_BUFFER_SIZE * bufferPtr],
                 read * backend->bytesPerSample, data->sampleRate);
    checkForAlErrors("alBufferData");
    if (!initial) {
        alSourceQueueBuffers(source->source, 1, &buffer);
        checkForAlErrors("alSourceQueueBuffers");
    }
    ++bufferPtr;
    bufferPtr %= NUM_BUFFERS;
}

int AudioStream::sampleRate() { return data ? data->sampleRate : 0; }

float AudioStream::currentPlayTime() {
    int sampleOffset;
    alGetSourcei(source->source, AL_SAMPLE_OFFSET, &sampleOffset);
    checkForAlErrors("currentPlayTime");
    return static_cast<float>(samplesPlayed + sampleOffset) / data->sampleRate;
}

void AudioStream::onLoop(const std::string &name) {
    auto music = engine->getMusic(name);
    onLoopData = music;
}

}
