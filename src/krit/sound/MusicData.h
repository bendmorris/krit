#ifndef KRIT_SOUND_MUSICDATA
#define KRIT_SOUND_MUSICDATA

#include <sndfile.h>
#include "krit/sound/AudioBackend.h"

namespace krit {

struct MusicData {
    int sampleRate;
    int channels;
    int frames;
    ALenum format;
    SNDFILE *sndFile = nullptr;
    VirtualIo io;

    MusicData() {}
    ~MusicData();
};

}

#endif
