#ifndef KRIT_BACKEND
#define KRIT_BACKEND

#include "krit/utils/Panic.h"
#include "krit/render/Gl.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <string>

namespace krit {

struct AudioBackend {
    ALCdevice *device = nullptr;
    ALCcontext *context = nullptr;
    ALuint source = 0, buffers[4];
    ALuint frequency = 0;
    ALenum format;
    unsigned char *buf;

    AudioBackend() {
        this->device = alcOpenDevice(nullptr);
        if (!this->device) {
            panic("couldn't open OpenAL device");
        }

        this->context = alcCreateContext(this->device, nullptr);
        if (!this->context) {
            panic("couldn't create OpenAL context");
        }
        if (!alcMakeContextCurrent(this->context)) {
            panic("couldn't set OpenAL context");
        }
    }

    ~AudioBackend() {
        alcMakeContextCurrent(NULL);
        alcDestroyContext(this->context);
        alcCloseDevice(this->device);
    }
};

}

#endif
