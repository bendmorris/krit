#include "krit/audio/AudioSource.h"
#include "krit/Engine.h"
#include "krit/audio/AudioBackend.h"
#include "krit/utils/Log.h"
#include "krit/utils/Profiling.h"
#include <AL/alext.h>
#include <cassert>
#include <cstring>
#include <sndfile.h>

namespace krit {

size_t AudioStream::bufferDurationMs{100};
static const int MIN_QUEUED_BUFFERS = AudioStream::NUM_BUFFERS;

std::vector<std::vector<char>> AudioStream::SampleRingBuffer::pool;

template <typename T, typename U, typename... Args>
static void childRender(T &me, U &child, Args... args) {
    float oldGain = child.gain;
    Vec3f oldPos = child.position;
    child.gain = oldGain * me.gain;
    child.position += me.position;
    child.render(args...);
    child.gain = oldGain;
    child.position = oldPos;
}

void AudioStream::reset(int position) {
    reachedEnd = false;
    sampleFramesRead = 0;
    sf_seek(cursor.sndFile.get(), position, SEEK_SET);
}

template <typename T> static T powerOfTwo(T v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    return ++v;
}

AudioStream::SampleRingBuffer::SampleRingBuffer(size_t streamBufferSize)
    : streamBufferSize(streamBufferSize) {}

AudioStream::SampleRingBuffer::~SampleRingBuffer() {
    if (!data.empty()) {
        pool.push_back(std::move(data));
    }
}

AudioStream::SampleRingBuffer
AudioStream::SampleRingBuffer::get(size_t streamBufferSize) {
    size_t desiredSize = streamBufferSize * NUM_BUFFERS;
    for (int i = pool.size() - 1; i >= 0; --i) {
        if (pool[i].size() == desiredSize) {
            if (i != static_cast<int>(pool.size() - 1)) {
                std::swap(pool[i], pool[pool.size() - 1]);
            }
            SampleRingBuffer result(streamBufferSize);
            result.data = std::move(pool.back());
            pool.pop_back();
            return result;
        }
    }
    SampleRingBuffer result(streamBufferSize);
    result.data.resize(desiredSize);
    return result;
}

void AudioStream::render(AudioBackend *backend, ALuint alSource) {
    ProfileZone("AudioStream::render");
    assert(backend);
    assert(alSource);
    if (reachedEnd) {
        return;
    }
    auto &audioData = cursor.audioData;
    assert(audioData);
    auto sndFile = cursor.sndFile.get();
    assert(sndFile);
    if (!sampleRingBuffer) {
        int streamBufferSize = powerOfTwo(
            audioData->channels * audioData->sampleRate *
            backend->bytesPerSample() * bufferDurationMs / NUM_BUFFERS / 1000);
        sampleRingBuffer = SampleRingBuffer::get(streamBufferSize);
        AREA_LOG_INFO("audio",
                      "created audio buffers for source %u of stream %s; "
                      "buffer size=%zux%i channels=%i",
                      alSource, audioData->path.c_str(), streamBufferSize,
                      NUM_BUFFERS, audioData->channels);
    }
    auto streamBufferSize = sampleRingBuffer->streamBufferSize;
    alSource3f(alSource, AL_POSITION, position.x(), position.y(), position.z());
    AREA_LOG_DEBUG("audio", "rendering stream %s as source %u with gain=%.2f",
                   audioData->path.c_str(), alSource, gain);
    // check if we've finished playing any of our buffers
    ALint buffersProcessed = 0;
    alGetSourcei(alSource, AL_BUFFERS_PROCESSED, &buffersProcessed);
    if (buffersProcessed) {
        AREA_LOG_DEBUG("audio", "unqueueing %i buffers", buffersProcessed);
        for (int i = 0; i < buffersProcessed; ++i) {
            ALuint alBuffer{0};
            alSourceUnqueueBuffers(alSource, 1, &alBuffer);
            engine->audio.recycleAlBuffer(alBuffer);
            checkForAlErrors("alSourceUnqueueBuffers");
        }
        bufferPtr += buffersProcessed;
        bufferPtr %= NUM_BUFFERS;
    }
    // if we need more buffers, render now
    alGetSourcei(alSource, AL_BUFFERS_QUEUED, &this->bufferCount);
    if (bufferCount < NUM_BUFFERS) {
        int bytesPerSampleFrame =
            audioData->channels * backend->bytesPerSample();
        int framesToRead =
            sampleRingBuffer->streamBufferSize / bytesPerSampleFrame;
        ALenum fmt;
        switch (audioData->channels) {
            case 1: {
                fmt = backend->streamFloats ? AL_FORMAT_MONO_FLOAT32
                                            : AL_FORMAT_MONO16;
                break;
            }
            case 2: {
                fmt = backend->streamFloats ? AL_FORMAT_STEREO_FLOAT32
                                            : AL_FORMAT_STEREO16;
                break;
            }
            default: {
                LOG_ERROR("unsupported channel count: %i", audioData->channels);
                assert(false);
                return;
            }
        }
        do {
            // render one buffer
            AREA_LOG_DEBUG("audio", "rendering one buffer");
            int buf = bufferIndex(bufferCount);
            int bytesRead = 0;
            if (gain <= 0) {
                // shortcut when we don't actually need the audio: seek forward
                // in audio data, then write zeroes
                bytesRead =
                    std::max<int>(0, sf_seek(sndFile, framesToRead, SEEK_CUR) -
                                         sampleFramesRead) *
                    bytesPerSampleFrame;
                std::memset(&sampleRingBuffer->data[streamBufferSize * buf], 0,
                            streamBufferSize);
                AREA_LOG_DEBUG("audio", "skipped %i bytes", bytesRead);
            } else {
                // decode one buffer
                std::memset(&sampleRingBuffer->data[streamBufferSize * buf], 0,
                            streamBufferSize);
                if (backend->streamFloats) {
                    bytesRead =
                        sf_readf_float(
                            sndFile,
                            (float *)(&sampleRingBuffer
                                           ->data[streamBufferSize * buf]),
                            framesToRead) *
                        bytesPerSampleFrame;
                    if (gain != 1) {
                        auto src =
                            (float *)(&sampleRingBuffer
                                           ->data[streamBufferSize * buf]);
                        float g = sqrt(gain);
                        for (size_t i = 0;
                             i < bytesRead / backend->bytesPerSample(); ++i) {
                            src[i] *= g;
                        }
                    }
                } else {
                    bytesRead =
                        sf_readf_short(
                            sndFile,
                            (int16_t *)(&sampleRingBuffer
                                             ->data[streamBufferSize * buf]),
                            framesToRead) *
                        bytesPerSampleFrame;
                    if (gain != 1) {
                        auto src =
                            (int16_t *)(&sampleRingBuffer
                                             ->data[streamBufferSize * buf]);
                        float g = sqrt(gain);
                        for (size_t i = 0;
                             i < bytesRead / backend->bytesPerSample(); ++i) {
                            src[i] = static_cast<float>(src[i]) * g;
                        }
                    }
                }
                AREA_LOG_DEBUG("audio", "decoded %i bytes", bytesRead);
            }
            if (bytesRead) {
                sampleFramesRead += bytesRead / bytesPerSampleFrame;
                ++bufferCount;
                ALuint alBuffer = engine->audio.getAlBuffer();
                assert(alBuffer);
                alBufferData(alBuffer, fmt,
                             &sampleRingBuffer->data[streamBufferSize * buf],
                             bytesRead, audioData->sampleRate);
                checkForAlErrors("alBufferData");
                alSourceQueueBuffers(alSource, 1, &alBuffer);
                checkForAlErrors("alSourceQueueBuffers");
            }
            if (bytesRead < framesToRead * bytesPerSampleFrame) {
                reachedEnd = true;
                break;
            }
        } while (bufferCount < MIN_QUEUED_BUFFERS);
    }
    AREA_LOG_DEBUG("audio", "done rendering; queued buffers: %zu", bufferCount);
    ALenum state;
    alGetSourcei(alSource, AL_SOURCE_STATE, &state);
    if (state != AL_PLAYING) {
        alSourcePlay(alSource);
        checkForAlErrors("aSourcePlay");
    }
}

void AudioSource::play() {
    switch (state) {
        case PlaybackState::Stopped: {
            AREA_LOG_INFO("audio", "play audio source");
            bool exists = false;
            auto shared = shared_from_this();
            for (auto &existing : engine->audio.active) {
                if (existing == shared) {
                    exists = true;
                    break;
                }
            }
            if (!exists) {
                engine->audio.play(shared);
            }
            state = PlaybackState::Playing;
            break;
        }
        case PlaybackState::Paused: {
            state = PlaybackState::Playing;
            break;
        }
        default: {
        }
    }
}

void AudioSource::stop() {
    if (state != PlaybackState::Stopped) {
        AREA_LOG_INFO("audio", "stop audio source");
        state = PlaybackState::Stopped;
        reset(true);
    }
}

void AudioSource::pause() { state = PlaybackState::Paused; }

StreamAudioSource::~StreamAudioSource() {
    if (alSource) {
        engine->audio.recycleAlSource(alSource);
    }
}

void StreamAudioSource::render(AudioBackend *backend) {
    ProfileZone("StreamAudioSource::update");
    if (!alSource) {
        alSource = engine->audio.getAlSource();
    }
    childRender(*this, stream, backend, alSource);
    if (stream.reachedEnd && loop) {
        stream.reset(loopFrom);
    }
}

void StreamAudioSource::update(AudioBackend *backend) {
    ProfileZone("StreamAudioSource::update");
    AREA_LOG_DEBUG("audio", "audio update");
    if (state != PlaybackState::Playing) {
        return;
    }
    render(backend);
    if (stream.reachedEnd && !loop) {
        stop();
    }
}

void StreamAudioSource::reset(bool clearSource) {
    AREA_LOG_DEBUG("audio", "reset audio source");
    stream.reset();
    if (clearSource && alSource) {
        engine->audio.recycleAlSource(alSource);
        alSource = 0;
    }
}

SequenceAudioSource::~SequenceAudioSource() {
    if (alSource) {
        engine->audio.recycleAlSource(alSource);
    }
}

void SequenceAudioSource::render(AudioBackend *backend) {
    ProfileZone("SequenceAudioSource::update");
    if (index >= static_cast<int>(parts.size())) {
        return;
    }
    if (!alSource) {
        alSource = engine->audio.getAlSource();
        assert(alSource);
    }
    auto &next = parts[index];
    childRender(*this, *next, backend, alSource);
    if (next->reachedEnd) {
        AREA_LOG_INFO("audio", "sequential audio progressing");
        if (++index >= static_cast<int>(parts.size())) {
            AREA_LOG_INFO("audio", "sequential audio looping");
            if (loop) {
                reset(false);
                index = loopStart;
            } else {
                stop();
            }
        }
    }
}

void SequenceAudioSource::reset(bool clearSource) {
    index = 0;
    for (auto &part : parts) {
        part->reset();
    }
    if (clearSource && alSource) {
        engine->audio.recycleAlSource(alSource);
        alSource = 0;
    }
}

std::shared_ptr<SequenceAudioSource>
SequenceAudioSource::addPart(std::shared_ptr<AudioData> data) {
    parts.emplace_back(new AudioStream(data));
    return std::static_pointer_cast<SequenceAudioSource>(shared_from_this());
}

void LayeredAudioSource::render(AudioBackend *backend) {
    ProfileZone("LayeredAudioSource::update");
    for (int i = layers.size() - 1; i >= 0; --i) {
        auto &child = layers[i];
        child->loop = loop;
        childRender(*this, *child, backend);
    }
    if (layers.empty()) {
        stop();
    }
}

void LayeredAudioSource::reset(bool clearSource) {
    for (auto &layer : layers) {
        layer->reset(clearSource);
    }
}

void LayeredAudioSource::select(size_t index) {
    for (size_t i = 0; i < layers.size(); ++i) {
        layers[i]->gain = i == index ? 1 : 0;
    }
}

std::shared_ptr<LayeredAudioSource>
LayeredAudioSource::addLayer(std::shared_ptr<AudioSource> source) {
    layers.emplace_back(source);
    return std::static_pointer_cast<LayeredAudioSource>(shared_from_this());
}
}
