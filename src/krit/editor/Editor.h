#ifndef KRIT_EDITOR_EDITOR
#define KRIT_EDITOR_EDITOR

#define IMGUI_DEFINE_MATH_OPERATORS
#include "krit/Sprite.h"
#include <GL/glew.h>
#include <SDL2/SDL_video.h>
#include <algorithm>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

struct ImGuiIO;

namespace krit {

struct ImageData;
struct RenderContext;

struct DevTool {
    virtual ~DevTool() {}
    virtual void draw(krit::RenderContext &ctx) {}
};

typedef std::function<void(RenderContext &)> MetricGetter;

struct Overlay : public DevTool {
    static std::vector<MetricGetter> metrics;

    static void addMetric(MetricGetter getter) {
        metrics.emplace_back(std::move(getter));
    };

    int fpsBuffer[4] = {0};
    int index = 0;
    float elapsed = 0;

    void draw(krit::RenderContext &ctx) override;
};

struct Editor : public Sprite {
    static bool imguiInitialized;
    static GLuint imguiTextureId;
    static SDL_Window *window;

    ImGuiIO *io;
    std::shared_ptr<ImageData> texture;

    Editor();

    void addDevTool(DevTool *tool) { devTools.emplace_back(tool); }

    void render(krit::RenderContext &) override;

private:
    std::vector<std::unique_ptr<DevTool>> devTools;
};

}

#endif
