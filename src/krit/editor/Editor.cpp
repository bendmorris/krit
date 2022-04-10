#if KRIT_ENABLE_TOOLS
#include "krit/editor/Editor.h"

#include "imgui.h"
#include "krit/App.h"
#include "krit/Camera.h"
#include "krit/Engine.h"
#include "krit/UpdateContext.h"
#include "krit/math/Dimensions.h"
#include "krit/render/CommandBuffer.h"
#include "krit/render/DrawCommand.h"
#include "krit/render/RenderContext.h"
#include "krit/utils/Memory.h"
#include <algorithm>

namespace krit {

static std::vector<Metric> defaultMetrics() {
    std::vector<Metric> v;
    v.emplace_back("Frame time",
                   [](RenderContext &ctx) { return ctx.elapsed * 1000; });
    return v;
}

std::vector<Metric> Overlay::metrics = defaultMetrics();

void Overlay::draw(krit::RenderContext &ctx) {
    ImGui::SetNextWindowPos(ImVec2(ctx.window->x() - 32, 32), ImGuiCond_Always, ImVec2(1, 0));
    bool pOpen;
    if (!ctx.engine->paused) {
        elapsed += ctx.elapsed;
    }
    if (ImGui::Begin("FPS", &pOpen,
                     ImGuiWindowFlags_NoDecoration |
                         ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Time: %.1f", elapsed);
        ImGui::Text("Memory: %.3f MB", getCurrentRss() / 1000000.0);
        ImGui::Text("Peak Mem: %.3f MB", getPeakRss() / 1000000.0);
        for (auto &metric : metrics) {
            metric.poll(ctx);
            if (metric.wrapped) {
                MetricStats stats = metric.getStats();
                ImGui::Text("%s: %.2f (%.2f-%.2f)", metric.name.c_str(),
                            stats.avg, stats.min, stats.max);
            }
        }
        ImGui::Checkbox("Debug draw", &ctx.debugDraw);
        ImGui::Checkbox("Pause", &ctx.engine->paused);
        if (ImGui::Button("Advance Frame")) {
            ctx.engine->paused = false;
            ctx.engine->setTimeout([](UpdateContext *ctx, void *) {
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
        // io.FontGlobalScale =
        //     std::max(1.0, ctx.window->height() * 1.0 / ctx.height());
        io.DisplaySize.x = ctx.window->x();
        io.DisplaySize.y = ctx.window->y();

        ImGui::NewFrame();

        for (auto &tool : devTools) {
            tool->draw(ctx);
        }

        ImGui::Render();

        // frame will be finished in the render thread
        ctx.drawCommandBuffer->buf.emplace_back<RenderImGui>(
            ImGui::GetDrawData());
    }
}
}
#endif
