#ifndef KRIT_BACKEND
#define KRIT_BACKEND

#include "krit/asset/AssetInfo.h"
#include "krit/render/Gl.h"
#include "krit/utils/Panic.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <memory>
#include <string>

namespace krit {

struct MusicData;
struct SoundData;
struct AudioStream;
struct AudioBackend;

struct AudioSource {
    ALuint source = 0;
    AudioSource *next = nullptr;
};

struct AudioStream {
    static const int NUM_BUFFERS = 4;

    bool playing = false;
    bool repeat = false;

    AudioStream(MusicData *data = nullptr) : data(data) {}

    void play();
    void pause();
    void stop();
    void feed(bool initial);
    void reset();

    float &getVolume() { return volume; }
    float setVolume(float v);

    void clear() {
        volume = 1;
        playing = false;
        repeat = false;
        data = nullptr;
        source = nullptr;
        bufferPtr = 0;
    }

private:
    std::unique_ptr<char[]> ringBuffer;
    AudioBackend *backend;
    MusicData *data = nullptr;
    AudioSource *source = nullptr;
    ALuint buffer[NUM_BUFFERS] = {0};
    int bufferPtr = 0;
    float volume = 1;

    friend struct AudioBackend;
};

struct VirtualIo {
    int length = 0;
    int cursor = 0;
    char *data;
};

struct AudioBackend {
    static const int MAX_SOURCES = 64;
    static const int MAX_STREAMS = 4;

    ALCdevice *device = nullptr;
    ALCcontext *context = nullptr;
    AudioSource *sourcePool = nullptr;
    AudioSource *activeSources = nullptr;

    AudioBackend();
    ~AudioBackend();

    void playSound(const std::string &name);
    void playSound(const AssetInfo &info);
    void playSound(SoundData *sound);

    AudioStream *playMusic(const std::string &name);
    AudioStream *playMusic(const AssetInfo &info);
    AudioStream *playMusic(MusicData *music);

    AudioSource *getSource();
    void recycleSource(AudioSource *source);

    void update();

private:
    AudioSource _sources[MAX_SOURCES];
    AudioStream _streams[MAX_STREAMS];
};

}

#endif
