#include "krit/RenderThread.h"
#include "krit/render/Gl.h"
#include "krit/utils/Panic.h"
#include "krit/editor/Editor.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl.h"

namespace krit {

void RenderThread::init() {
    // SDL_GL
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, GL_TRUE);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    this->glContext = SDL_GL_CreateContext(this->window);
    if (!this->glContext) {
        panic(SDL_GetError());
    }
    SDL_GL_MakeCurrent(this->window, this->glContext);
    // // try to get adaptive vsync
    // int result = SDL_GL_SetSwapInterval(-1);
    // fall back to regular vsync
    // if (result == -1) {
        SDL_GL_SetSwapInterval(1);
    // }
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        panic("%s\n", glewGetErrorString(err));
    }
    checkForGlErrors("glew init");

    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init(nullptr);
    checkForGlErrors("render thread init");

    auto &io = ImGui::GetIO();

    unsigned char* pixels = nullptr;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    glActiveTexture(GL_TEXTURE0);
    checkForGlErrors("imgui active texture");
    glGenTextures(1, &Editor::imguiTextureId);
    checkForGlErrors("imgui gen textures");
    glBindTexture(GL_TEXTURE_2D, Editor::imguiTextureId);
    checkForGlErrors("imgui bind texture");
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    checkForGlErrors("imgui texImage2D");

    io.Fonts->TexID = (void*)Editor::imguiTextureId;
    Editor::imguiInitialized = true;
}

void RenderThread::renderLoop() {
    init();

    while (true) {
        TaskManager::work(taskManager.renderQueue, render);
        SDL_LockMutex(renderCondMutex);
        bool signaled = !SDL_CondWaitTimeout(renderCond, renderCondMutex, 100);
        SDL_UnlockMutex(renderCondMutex);

        if (killed) {
            SDL_DestroyCond(renderCond);
            SDL_DestroyMutex(renderMutex);
            SDL_DestroyMutex(renderCondMutex);
            break;
        }
        if (!signaled) {
            continue;
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
