#ifndef KRIT_SOUND_SOUNDDATA
#define KRIT_SOUND_SOUNDDATA

#include <sndfile.h>
#include <AL/al.h>

namespace krit {

struct SoundData {
    int sampleRate;
    int channels;
    int frames;
    ALuint buffer = 0;

    ~SoundData();
};

}

#endif
