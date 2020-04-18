#ifndef KRIT_SOUND_SOUNDCONTEXT
#define KRIT_SOUND_SOUNDCONTEXT

#include "krit/asset/AssetContext.h"
#include <string>

namespace krit {

struct SoundContext {
    AssetContext &asset;

    SoundContext(AssetContext &asset): asset(asset) {}

    void playSound(std::string &name);
};

}

#endif
