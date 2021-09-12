#ifndef KRIT_SOUND_SOUNDDATA
#define KRIT_SOUND_SOUNDDATA

#include <sndfile.h>

namespace krit {

struct SoundData {
    int16_t *data;
    int sampleRate;
    int channels;
    int frames;
};

}

#endif
