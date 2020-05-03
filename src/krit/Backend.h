#ifndef KRIT_BACKEND
#define KRIT_BACKEND

#include "krit/utils/Panic.h"
#include "krit/render/Gl.h"
#include <SDL.h>
#include <SDL_image.h>
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

struct SdlBackend {
    SDL_Window *window;
    SDL_Surface *surface;
    // AudioBackend audio;

    SdlBackend(std::string title, int width, int height);

    void getWindowSize(int *w, int *h) {
        SDL_GetWindowSize(this->window, w, h);
    }

    void setFullScreen(bool full) {
        SDL_SetWindowFullscreen(this->window, full ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    }

    ~SdlBackend() {
        SDL_DestroyWindow(this->window);
    }
};

typedef SdlBackend Backend;

}

#endif
