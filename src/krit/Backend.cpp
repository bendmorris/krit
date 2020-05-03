#include "krit/Backend.h"
#include "krit/render/Gl.h"

namespace krit {

SdlBackend::SdlBackend(std::string title, int width, int height) {
    // SDL
    SDL_Init(SDL_INIT_VIDEO);

    // SDL_Image
    IMG_Init(IMG_INIT_PNG);

    this->window = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        width, height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );
    if (!this->window) {
        panic(SDL_GetError());
    }

    this->surface = SDL_GetWindowSurface(this->window);

    checkForGlErrors("create backend");
}

}
