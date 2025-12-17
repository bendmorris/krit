#ifndef KRIT_AUDIO_AUDIOSOURCE
#define KRIT_AUDIO_AUDIOSOURCE

#include "krit/audio/AudioData.h"
#include "krit/math/Vec.h"
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

namespace krit {

struct AudioBackend;

/**
 * A single streaming audio source.
 */
struct AudioStream {
    static const int NUM_BUFFERS = 8;

    struct SampleRingBuffer {
        static std::vector<std::vector<char>> pool;
        static SampleRingBuffer get(size_t streamBufferSize);

        SampleRingBuffer(size_t streamBufferSize);
        ~SampleRingBuffer();

        size_t streamBufferSize;
        std::vector<char> data;
    };

    static size_t bufferDurationMs;

    AudioStream(std::shared_ptr<AudioData> data) : cursor(data) {}

    Vec3f position;
    float gain{1};

    AudioDataCursor cursor;
    bool reachedEnd{false};

    void render(AudioBackend *backend, ALuint alSource);
    void reset(int position = 0);

    size_t bufferIndex(size_t index) {
        return (bufferPtr + index) % NUM_BUFFERS;
    }

    std::optional<SampleRingBuffer> sampleRingBuffer;
    size_t bufferPtr{0};
    int bufferCount{0};
    int sampleFramesRead{0};
};

/**
 * Abstract class for audio sources.
 */
struct AudioSource : public std::enable_shared_from_this<AudioSource> {
    enum class PlaybackState {
        Stopped,
        Paused,
        Playing,
    };

    virtual ~AudioSource() {};

    PlaybackState state{PlaybackState::Stopped};
    Vec3f position;
    float gain{1};
    bool loop{false};
    int loopFrom{0};

    void play();
    void stop();
    void pause();
    virtual void reset(bool clearSource) = 0;
    virtual void render(AudioBackend *backend) = 0;
    virtual void update(AudioBackend *backend) {
        if (state == PlaybackState::Playing) {
            this->render(backend);
        }
    };

    bool playing() { return state == PlaybackState::Playing; }
};

struct StreamAudioSource : public AudioSource {
    AudioStream stream;
    ALuint alSource{0};

    StreamAudioSource(std::shared_ptr<AudioData> data) : stream(data) {}
    ~StreamAudioSource();
    void reset(bool clearSource) override;
    void render(AudioBackend *backend) override;
    void update(AudioBackend *backend) override;
};

/**
 * A horizontal vector of audio sources, which will be played sequentially.
 */
struct SequenceAudioSource : public AudioSource {
    std::vector<std::shared_ptr<AudioStream>> parts;
    ALuint alSource{0};
    int loopStart{0};
    int index{0};

    ~SequenceAudioSource();
    void reset(bool clearSource) override;
    void render(AudioBackend *backend) override;

    std::shared_ptr<SequenceAudioSource> addPart(std::shared_ptr<AudioData>);
};

/**
 * A vertical vector of audio sources, which can be arbitrarily mixed.
 */
struct LayeredAudioSource : public AudioSource {
    std::vector<std::shared_ptr<AudioSource>> layers;

    void reset(bool clearSource) override;
    void render(AudioBackend *backend) override;

    void select(size_t index);
    std::shared_ptr<LayeredAudioSource> addLayer(std::shared_ptr<AudioSource>);
};

}

#endif
