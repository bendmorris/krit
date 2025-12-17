#ifndef KRIT_BACKEND
#define KRIT_BACKEND

#include "krit/render/Gl.h"
#include "krit/utils/Panic.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <array>
#include <memory>
#include <string>
#include <vector>

#if KRIT_SOUND_THREAD
#include <mutex>
#include <thread>
#endif

namespace krit {

#if KRIT_RELEASE
#define checkForAlErrors(...)
#else
#define STRINGIZE_DETAIL(x) #x
#define STRINGIZE(x) STRINGIZE_DETAIL(x)
#define checkForAlErrors(...)                                                  \
    _checkForAlErrors("AL Error at " __FILE__                                  \
                      ":" STRINGIZE(__LINE__) ": " __VA_ARGS__)
void _checkForAlErrors(const char *fmt, ...);
#endif

struct AudioData;
struct AudioSource;
struct StreamAudioSource;
struct SequenceAudioSource;
struct LayeredAudioSource;

struct AudioBackend {
    bool enabled = false;
    ALCdevice *device = nullptr;
    ALCcontext *context = nullptr;
    AudioSource *sourcePool = nullptr;
    AudioSource *activeSources = nullptr;
    float gain = 1;
    bool streamFloats = false;

    std::vector<std::shared_ptr<AudioSource>> active;

    AudioBackend();
    ~AudioBackend();

    std::shared_ptr<StreamAudioSource> oneShot(std::shared_ptr<AudioData>);
    std::shared_ptr<StreamAudioSource> loop(std::shared_ptr<AudioData>);
    std::shared_ptr<SequenceAudioSource> sequence();
    std::shared_ptr<LayeredAudioSource> layered();

    ALuint getAlSource();
    void recycleAlSource(ALuint);

    ALuint getAlBuffer();
    void recycleAlBuffer(ALuint);

    size_t bytesPerSample() {
        return streamFloats ? sizeof(float) : sizeof(int16_t);
    };
    void init();
    void update();
    void refreshDefaultDevice();
    void play(std::shared_ptr<AudioSource>);

#if KRIT_SOUND_THREAD
    bool soundThreadJoined{false};
#endif

private:
    ALCboolean(ALC_APIENTRY *alcReopenDeviceSOFT)(
        ALCdevice *device, const ALCchar *name,
        const ALCint *attribs) = nullptr;
    ALCboolean(ALC_APIENTRY *alcResetDeviceSOFT)(
        ALCdevice *device, const ALCint *attribs) = nullptr;
    bool needToRefreshDefaultDevice{false};
    std::vector<ALuint> alSourcePool;
    std::vector<ALuint> alSourceQuarantine;
    std::vector<ALuint> alBufferPool;

#if KRIT_SOUND_THREAD
    std::vector<std::shared_ptr<AudioSource>> _currentActive;
    std::thread soundThread;
    std::mutex soundThreadMutex;
#endif
};

}

#endif
