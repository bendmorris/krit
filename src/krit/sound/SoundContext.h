#ifndef KRIT_SOUND_SOUNDCONTEXT
#define KRIT_SOUND_SOUNDCONTEXT

#include <string>

namespace krit {

struct SoundContext {
    SoundContext() {}

    void playSound(const std::string &name);
};

}

#endif
