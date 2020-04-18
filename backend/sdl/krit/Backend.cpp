#include "krit/Backend.h"
#include "krit/render/Gl.h"

namespace krit {

SdlBackend::SdlBackend(std::string title, int width, int height) {
    // SDL
    SDL_Init(SDL_INIT_VIDEO);

    // SDL_GL
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, GL_TRUE);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    glEnable(GL_MULTISAMPLE);

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

    this->glContext = SDL_GL_CreateContext(this->window);
    if (!this->glContext) {
        panic(SDL_GetError());
    }
    this->surface = SDL_GetWindowSurface(this->window);

    SDL_GL_MakeCurrent(this->window, this->glContext);
    SDL_GL_SetSwapInterval(1);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    checkForGlErrors("create backend");
}

}
