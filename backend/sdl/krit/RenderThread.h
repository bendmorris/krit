#include "krit/render/RenderContext.h"
#include "krit/App.h"
#include "krit/TaskManager.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_opengl.h"

namespace krit {

struct RenderThread {
    UpdateContext &update;
    RenderContext &render;
    TaskManager &taskManager;
    SDL_GLContext glContext;
    SDL_Thread *thread;
    SDL_mutex *renderMutex;
    SDL_mutex *renderCondMutex;
    SDL_cond *renderCond;
    SDL_Window *window;
    bool killed = false;

    RenderThread(UpdateContext &update, RenderContext &render, TaskManager &taskManager, SDL_Window *window):
        update(update),
        render(render),
        taskManager(taskManager),
        window(window)
    {
        renderMutex = SDL_CreateMutex();
        renderCondMutex = SDL_CreateMutex();
        renderCond = SDL_CreateCond();
        SDL_Thread *renderThread = SDL_CreateThread(RenderThread::exec, "render", this);
    }

    static int exec(void *raw) {
        static_cast<RenderThread*>(raw)->renderLoop();
        return 0;
    }

    void init();

    void renderLoop() {
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
        }
    }
};

}
