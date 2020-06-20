#include "krit/render/RenderContext.h"
#include "krit/render/Gl.h"
#include "krit/App.h"
#include "krit/TaskManager.h"
#include <SDL.h>

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
        thread = SDL_CreateThread(RenderThread::exec, "render", this);
    }

    static int exec(void *raw) {
        static_cast<RenderThread*>(raw)->renderLoop();
        return 0;
    }

    void init();

    void renderLoop();
};

}
