#include "krit/audio/AudioBackend.h"
#include "krit/Engine.h"
#include "krit/audio/AudioData.h"
#include "krit/audio/AudioSource.h"
#include "krit/utils/Log.h"
#include "krit/utils/Profiling.h"
#include "krit/utils/ScopedMutex.h"
#include <AL/alext.h>
#include <sndfile.h>

#if KRIT_SOUND_THREAD
#include <chrono>
#endif

namespace krit {

void _checkForAlErrors(const char *fmt, ...) {
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

static void ALC_APIENTRY _openalEvent(ALCenum eventType, ALCenum deviceType,
                                      ALCdevice *device, ALCsizei length,
                                      const ALCchar *message,
                                      void *userParam) noexcept {
    AREA_LOG_INFO("audio", "OpenAL system event received: %i", eventType);
    AudioBackend *a = static_cast<AudioBackend *>(userParam);
    switch (eventType) {
        case ALC_EVENT_TYPE_DEFAULT_DEVICE_CHANGED_SOFT: {
            AREA_LOG_INFO("audio", "handling default audio device change");
            a->refreshDefaultDevice();
            break;
        }
        default: {
        }
    }
}

#if KRIT_SOUND_THREAD
namespace {
void runSoundThread(AudioBackend *backend) {
    backend->init();

    while (!backend->soundThreadJoined) {
        backend->update();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}
}
#endif

AudioBackend::AudioBackend() {
    device = alcOpenDevice(nullptr);
    if (!device) {
        AREA_LOG_FATAL("audio", "couldn't open OpenAL default device");
        return;
    }

    init();
#if KRIT_SOUND_THREAD
    soundThread = std::thread(runSoundThread, this);
#endif
}

AudioBackend::~AudioBackend() {
    // ALuint __sources[MAX_SOURCES] = {0};
    // for (int i = 0; i < MAX_SOURCES; ++i) {
    //     __sources[i] = _sources[i].source;
    // }
    // alDeleteSources(MAX_SOURCES, __sources);
    // for (int i = 0; i < MAX_STREAMS; ++i) {
    //     alDeleteBuffers(AudioStream::NUM_BUFFERS, &_streams[i].buffer[0]);
    // }
#if KRIT_SOUND_THREAD
    soundThreadJoined = true;
    soundThread.join();
#endif
    alcMakeContextCurrent(nullptr);
    if (!alBufferPool.empty()) {
        alDeleteBuffers(alBufferPool.size(), alBufferPool.data());
    }
    if (!alSourcePool.empty()) {
        alDeleteSources(alSourcePool.size(), alSourcePool.data());
    }
    if (!alSourceQuarantine.empty()) {
        alDeleteSources(alSourceQuarantine.size(), alSourceQuarantine.data());
    }
    if (context) {
        alcDestroyContext(context);
    }
    if (device) {
        alcCloseDevice(device);
    }
}

void AudioBackend::init() {
    if (alcIsExtensionPresent(device, "ALC_SOFT_reopen_device") &&
        alcIsExtensionPresent(device, "ALC_SOFT_system_events")) {
        auto alcEventIsSupportedSOFT = reinterpret_cast<ALCenum(ALC_APIENTRY *)(
            ALCenum eventType, ALCenum deviceType)>(
            alcGetProcAddress(device, "alcEventIsSupportedSOFT"));
        if (alcEventIsSupportedSOFT(ALC_EVENT_TYPE_DEFAULT_DEVICE_CHANGED_SOFT,
                                    ALC_PLAYBACK_DEVICE_SOFT) ==
            ALC_EVENT_SUPPORTED_SOFT) {
            // set up notifications to handle default device change
            alcReopenDeviceSOFT = reinterpret_cast<ALCboolean(ALC_APIENTRY *)(
                ALCdevice * device, const ALCchar *name,
                const ALCint *attribs)>(
                alcGetProcAddress(device, "alcReopenDeviceSOFT"));
            alcResetDeviceSOFT = reinterpret_cast<ALCboolean(ALC_APIENTRY *)(
                ALCdevice * device, const ALCint *attribs)>(
                alcGetProcAddress(device, "alcResetDeviceSOFT"));
            auto alcEventControlSOFT =
                reinterpret_cast<ALCboolean(ALC_APIENTRY *)(
                    ALCsizei count, const ALCenum *events, ALCboolean enable)>(
                    alcGetProcAddress(device, "alcEventControlSOFT"));
            auto alcEventCallbackSOFT = reinterpret_cast<void(ALC_APIENTRY *)(
                ALCEVENTPROCTYPESOFT callback, void *userParam)>(
                alcGetProcAddress(device, "alcEventCallbackSOFT"));
            ALCenum events[1] = {ALC_EVENT_TYPE_DEFAULT_DEVICE_CHANGED_SOFT};
            if (alcEventControlSOFT(1, events, ALC_TRUE) == ALC_TRUE) {
                AREA_LOG_INFO("audio",
                              "setting default audio device change callback");
                alcEventCallbackSOFT(_openalEvent, this);
            } else {
                AREA_LOG_INFO(
                    "audio",
                    "couldn't start listening for default audio device "
                    "change events");
            }
        } else {
            AREA_LOG_INFO("audio",
                          "default audio device change event is not supported");
        }
    } else {
        AREA_LOG_INFO("audio",
                      "OpenAL system events/device reopen are not supported");
    }

    ALCint attrs[] = {ALC_HRTF_SOFT, ALC_FALSE, 0};
    context = alcCreateContext(device, attrs);
    if (!context) {
        AREA_LOG_INFO("audio", "couldn't create OpenAL context");
        return;
    }
    if (!alcMakeContextCurrent(context)) {
        AREA_LOG_INFO("audio", "couldn't set OpenAL context");
        return;
    }
    checkForAlErrors("initial");

    streamFloats = alIsExtensionPresent("AL_EXT_float32");

    // ALuint __sources[MAX_SOURCES] = {0};
    // alGenSources(MAX_SOURCES, __sources);
    // checkForAlErrors("alGenSources");
    // sourcePool = &_sources[0];
    // for (int i = 0; i < MAX_SOURCES; ++i) {
    //     _sources[i].source = __sources[i];
    //     if (i > 0) {
    //         _sources[i - 1].next = &_sources[i];
    //     }
    // }

    enabled = true;
}

ALuint AudioBackend::getAlSource() {
    if (alSourcePool.empty()) {
        alSourcePool.resize(4);
        alGenSources(4, alSourcePool.data());
        checkForAlErrors("alGenSources");
        for (size_t i = 0; i < 4; ++i) {
            ALuint alSource = alSourcePool[i];
            assert(alSource);
            alSourcei(alSource, AL_LOOPING, AL_FALSE);
            alSourcei(alSource, AL_DISTANCE_MODEL, AL_LINEAR_DISTANCE_CLAMPED);
            alSourcef(alSource, AL_MAX_DISTANCE, 2.f);
            alSourcef(alSource, AL_REFERENCE_DISTANCE, 0.1f);
        }
    }
    ALuint alSource = alSourcePool.back();
    alSourcePool.pop_back();
    alSource3f(alSource, AL_POSITION, 0.f, 0.f, 0.f);
    return alSource;
}

void AudioBackend::recycleAlSource(ALuint alSource) {
    AREA_LOG_DEBUG("audio", "quarantine source %i", alSource);
    alSourceQuarantine.push_back(alSource);
}

ALuint AudioBackend::getAlBuffer() {
    if (alBufferPool.empty()) {
        AREA_LOG_DEBUG("audio", "generating buffers");
        alBufferPool.resize(8);
        alGenBuffers(8, alBufferPool.data());
        checkForAlErrors("alGenBuffers");
    }
    ALuint alBuffer = alBufferPool.back();
    alBufferPool.pop_back();
    AREA_LOG_DEBUG("audio", "get buffer: returning %i", alBuffer);
    return alBuffer;
}

void AudioBackend::recycleAlBuffer(ALuint buffer) {
    AREA_LOG_DEBUG("audio", "recycling buffer %i", buffer);
    alBufferPool.push_back(buffer);
}

void AudioBackend::refreshDefaultDevice() { needToRefreshDefaultDevice = true; }

void AudioBackend::update() {
    ProfileZone("AudioBackend::update");

#if KRIT_SOUND_THREAD
    {
    ScopedMutex _lock(&soundThreadMutex);
#endif
    if (alcReopenDeviceSOFT && needToRefreshDefaultDevice) {
        AREA_LOG_INFO("audio", "reopening default audio device");
        if (!alcReopenDeviceSOFT(device, nullptr, nullptr)) {
            AREA_LOG_INFO("audio", "failed to reopen default audio device");
        }
        if (alcResetDeviceSOFT) {
            alcResetDeviceSOFT(device, nullptr);
        }
        needToRefreshDefaultDevice = false;
    }
    for (int i = alSourceQuarantine.size() - 1; i >= 0; --i) {
        ALuint alSource = alSourceQuarantine[i];
        ALint buffersProcessed = 0;
        alGetSourcei(alSource, AL_BUFFERS_PROCESSED, &buffersProcessed);
        if (buffersProcessed) {
            AREA_LOG_DEBUG("audio",
                           "quarantined source %i processed %i buffers",
                           alSource, buffersProcessed);
            for (; buffersProcessed > 0; --buffersProcessed) {
                ALuint alBuffer{0};
                alSourceUnqueueBuffers(alSource, 1, &alBuffer);
                recycleAlBuffer(alBuffer);
            }
        }
        ALint buffers{0};
        alGetSourcei(alSource, AL_BUFFERS_QUEUED, &buffers);
        if (buffers == 0) {
            AREA_LOG_DEBUG("audio",
                           "quarantined source %i is ready to be recycled",
                           alSource);
            alSourcePool.push_back(alSource);
            alSourceQuarantine.erase(alSourceQuarantine.begin() + i);
        }
    }
#if KRIT_SOUND_THREAD
    }
    {
        ScopedMutex _lock(&soundThreadMutex);
        _currentActive = active;
    }
    auto &active = _currentActive;
#endif
    for (int i = active.size() - 1; i >= 0; --i) {
        if (active[i]->state == AudioSource::PlaybackState::Stopped) {
#if !KRIT_SOUND_THREAD
            AREA_LOG_DEBUG("audio", "remove stopped audio source");
            active.erase(active.begin() + i);
#endif
        } else {
            active[i]->update(this);
        }
    }
#if KRIT_SOUND_THREAD
    _currentActive.clear();
    {
        ScopedMutex _lock(&soundThreadMutex);
        for (int i = active.size() - 1; i >= 0; --i) {
            if (active[i]->state == AudioSource::PlaybackState::Stopped) {
                AREA_LOG_DEBUG("audio", "remove stopped audio source");
                active.erase(active.begin() + i);
            }
        }
    }
#endif
    AREA_LOG_DEBUG("audio", "audio update complete");
}

std::shared_ptr<StreamAudioSource>
AudioBackend::oneShot(std::shared_ptr<AudioData> data) {
    auto source = std::make_shared<StreamAudioSource>(data);
    source->loop = false;
    return source;
}
std::shared_ptr<StreamAudioSource>
AudioBackend::loop(std::shared_ptr<AudioData> data) {
    auto source = std::make_shared<StreamAudioSource>(data);
    source->loop = true;
    // check for a loop_start cue
    // uint32_t cue_count;
    // if (sf_command(source->stream.cursor.sndFile.get(), SFC_GET_CUE_COUNT,
    //                &cue_count, sizeof(cue_count)) == SF_TRUE) {
    //     SF_CUES cues;
    //     sf_command(source->stream.cursor.sndFile.get(), SFC_GET_CUE, &cues,
    //                sizeof(cues));
    //     for (uint32_t i = 0; i < cue_count; ++i) {
    //         if (!strncmp(cues.cue_points[i].name, "loop_start", 10)) {
    //             source->loopFrom = cues.cue_points[i].position;
    //             break;
    //         }
    //     }
    // }
    return source;
}
std::shared_ptr<SequenceAudioSource> AudioBackend::sequence() {
    auto source = std::make_shared<SequenceAudioSource>();
    source->loop = true;
    return source;
}
std::shared_ptr<LayeredAudioSource> AudioBackend::layered() {
    auto source = std::make_shared<LayeredAudioSource>();
    source->loop = true;
    return source;
}

void AudioBackend::play(std::shared_ptr<AudioSource> source) {
#if KRIT_SOUND_THREAD
    ScopedMutex _lock(&soundThreadMutex);
#endif
    active.push_back(source);
}

}
