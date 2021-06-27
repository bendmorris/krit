#include "krit/editor/Editor.h"
#include "krit/App.h"
#include "krit/Engine.h"
#include "krit/Utils.h"
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

namespace krit {

std::vector<MetricGetter> Overlay::metrics;

void Overlay::draw(krit::RenderContext &ctx) {
    ImVec2 window_pos = ImVec2(ctx.window->width() - 32, 32);
    ImVec2 window_pos_pivot = ImVec2(1, 0);
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    bool pOpen;
    if (!ctx.engine->paused) {
        elapsed += ctx.elapsed;
    }
    if (ImGui::Begin("FPS", &pOpen, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize)) {
        int next = (index++) % 4;
        fpsBuffer[next] = 1.0 / ctx.elapsed;
        float total = 0;
        for (int i = 0; i < 4; ++i) {
            total += fpsBuffer[next];
        }
        ImGui::Text("FPS: %.2f", total / 4);
        ImGui::Text("Time: %.1f", elapsed);
        ImGui::Text("Memory: %.3f MB", getCurrentRss() / 1000000.0);
        ImGui::Text("Peak Mem: %.3f MB", getPeakRss() / 1000000.0);
        for (auto &it : metrics) {
            it(ctx);
        }
        ImGui::Checkbox("Debug draw", &ctx.debugDraw);
        ImGui::Checkbox("Pause", &ctx.engine->paused);
        if (ImGui::Button("Advance Frame")) {
            ctx.engine->paused = false;
            ctx.engine->setTimeout([](UpdateContext *ctx, void*) {
                ctx->engine->paused = true;
                return false;
            });
        }
    }
    ImGui::End();
}

bool Editor::imguiInitialized = false;
GLuint Editor::imguiTextureId = 0;
SDL_Window *Editor::window = nullptr;

Editor::Editor() {
    devTools.push_back(std::unique_ptr<DevTool>(new Overlay()));
}

void Editor::render(krit::RenderContext &ctx) {
    if (imguiInitialized) {
        auto &io = ImGui::GetIO();
        io.DeltaTime = ctx.elapsed;
        io.FontGlobalScale = std::max(1.0, ctx.window->height() * 2.5 / ctx.camera->height());
        io.DisplaySize.x = ctx.window->width();
        io.DisplaySize.y = ctx.window->height();

        ImGui::NewFrame();

        for (auto &tool : devTools) {
            tool->draw(ctx);
        }

        ImGui::Render();

        // frame will be finished in the render thread
        window = ctx.app->window;
        ctx.drawCommandBuffer->buf.emplace_back<RenderImGui>(ImGui::GetDrawData());
    }
}

}
