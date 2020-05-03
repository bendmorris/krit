#include "krit/editor/Editor.h"
#include "krit/App.h"
#include "krit/Engine.h"
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

namespace krit {

void Overlay::draw(krit::RenderContext &ctx) {
    ImVec2 window_pos = ImVec2(32, 32);
    ImVec2 window_pos_pivot = ImVec2(0, 0);
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    bool pOpen;
    if (ImGui::Begin("FPS", &pOpen, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize)) {
        int next = (index++) % 4;
        fpsBuffer[next] = 1.0 / ctx.elapsed;
        double total = 0;
        for (int i = 0; i < 4; ++i) {
            total += fpsBuffer[next];
        }
        ImGui::Text("FPS: %.2f", total / 4);
    }
    ImGui::End();
}

bool Editor::imguiInitialized = false;
GLuint Editor::imguiTextureId = 0;
SDL_Window *Editor::window = nullptr;

Editor::Editor() {
    devTools.push_back(std::unique_ptr<DevTool>(new Overlay()));
}

void Editor::update(krit::UpdateContext &ctx) {}

void Editor::render(krit::RenderContext &ctx) {
    if (imguiInitialized) {
        auto &io = ImGui::GetIO();
        io.DeltaTime = ctx.elapsed;
        io.DisplaySize.x = ctx.window->width();
        io.DisplaySize.y = ctx.window->height();

        bool show_demo_window = true;
        ImGui::NewFrame();

        for (auto &tool : devTools) {
            tool->draw(ctx);
        }

        ImGui::Render();

        // frame will be finished in the render thread
        window = ctx.app->backend.window;
        ctx.drawCommandBuffer->emplace_back<RenderImGui>(ImGui::GetDrawData());
    }
}

}