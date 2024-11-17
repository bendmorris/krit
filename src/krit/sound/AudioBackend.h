#ifndef KRIT_BACKEND
#define KRIT_BACKEND

#include "krit/render/Gl.h"
#include "krit/utils/Panic.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <array>
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
    static const int NUM_BUFFERS = 8;

    bool repeat = false;
    bool playing = false;

    AudioStream(std::shared_ptr<MusicData> data = nullptr) : data(data) {}

    void play();
    void pause();
    void stop();
    void feed(bool initial);
    void reset();

    // void seek(float time);
    float &getVolume() { return volume; }
    float setVolume(float v);

    int sampleRate();
    float currentPlayTime();

    void onLoop(const std::string &name);
    // void onLoop(std::shared_ptr<MusicData> music);

    void clear() {
        volume = 1;
        repeat = false;
        data = nullptr;
        onLoopData = nullptr;
        source = nullptr;
        bufferPtr = 0;
        playing = false;
    }

private:
    std::unique_ptr<char[]> ringBuffer;
    AudioBackend *backend { nullptr };
    std::shared_ptr<MusicData> data;
    std::shared_ptr<MusicData> onLoopData;
    AudioSource *source { nullptr };
    std::array<ALuint, NUM_BUFFERS> buffer { 0 };
    int bufferPtr { 0 };
    int samplesPlayed { 0 };
    float volume { 1 };

    friend struct AudioBackend;
};

struct VirtualIo {
    int cursor = 0;
    std::string data;
};

struct AudioBackend {
    static const int MAX_SOURCES = 64;
    static const int MAX_STREAMS = 4;

    bool enabled = false;
    ALCdevice *device = nullptr;
    ALCcontext *context = nullptr;
    AudioSource *sourcePool = nullptr;
    AudioSource *activeSources = nullptr;
    float soundVolume = 1;
    size_t bytesPerSample = 0;
    bool streamFloats = false;

    AudioBackend();
    ~AudioBackend();

    void playSound(const std::string &name);
    void playSoundAsset(SoundData *sound);

    AudioStream *playMusic(const std::string &name);
    AudioStream *playMusicAsset(std::shared_ptr<MusicData> music);

    AudioSource *getSource();
    void recycleSource(AudioSource *source);

    void update();

private:
    std::array<AudioSource, MAX_SOURCES> _sources;
    std::array<AudioStream, MAX_STREAMS> _streams;
};

}

#endif
