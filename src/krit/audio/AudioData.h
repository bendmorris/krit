#ifndef KRIT_AUDIO_AUDIODATA
#define KRIT_AUDIO_AUDIODATA

#include <AL/al.h>
#include <memory>
#include <sndfile.h>
#include <string>

namespace krit {

struct AudioData {
    std::string path;
    std::string data;
    int sampleRate{0};
    int channels{0};
    int frames{0};
    ALenum format{0};
};

struct AudioDataCursor {
    std::shared_ptr<AudioData> audioData;
    size_t cursor{0};
    size_t size{0};
    std::unique_ptr<SNDFILE, decltype(&sf_close)> sndFile;

    AudioDataCursor(std::shared_ptr<AudioData> data);
};

}

#endif
