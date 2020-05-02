#include "krit/RenderThread.h"
#include "krit/render/Gl.h"
#include "krit/utils/Panic.h"
#include "SDL2/SDL_opengl.h"

namespace krit {

void RenderThread::init() {
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

    this->glContext = SDL_GL_CreateContext(this->window);
    if (!this->glContext) {
        panic(SDL_GetError());
    }
    SDL_GL_MakeCurrent(this->window, this->glContext);
    // try to get adaptive vsync
    int result = SDL_GL_SetSwapInterval(-1);
    // fall back to regular vsync
    if (result == -1) {
        SDL_GL_SetSwapInterval(1);
    }
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
}

void RenderThread::renderLoop() {
    init();

    while (true) {
        TaskManager::work(taskManager.renderQueue, render);
        SDL_LockMutex(renderCondMutex);
        SDL_CondWait(renderCond, renderCondMutex);
        SDL_UnlockMutex(renderCondMutex);

        if (killed) {
            SDL_DestroyCond(renderCond);
            SDL_DestroyMutex(renderMutex);
            SDL_DestroyMutex(renderCondMutex);
            break;
        }
        render.app->renderer.startFrame(render);
        SDL_LockMutex(renderMutex);
        render.app->renderer.flushBatch(render);
        render.app->renderer.flushFrame(render);
        SDL_UnlockMutex(renderMutex);

        SDL_LockMutex(renderCondMutex);
        SDL_CondSignal(renderCond);
        SDL_UnlockMutex(renderCondMutex);
    }
}

}
