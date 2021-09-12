#ifndef KRIT_BACKEND
#define KRIT_BACKEND

#include "krit/asset/AssetInfo.h"
#include "krit/render/Gl.h"
#include "krit/utils/Panic.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <string>

namespace krit {

struct SoundData;

struct AudioBackend {
    ALCdevice *device = nullptr;
    ALCcontext *context = nullptr;
    ALuint source = 0, buffers[4];
    ALuint frequency = 0;
    ALenum format;
    unsigned char *buf;

    AudioBackend();
    ~AudioBackend();

    void playSound(const std::string &name);
    void playSound(const AssetInfo &info);
    void playSound(SoundData *sound);
};

}

#endif
