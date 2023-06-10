#ifndef KRIT_EDITOR_EDITOR
#define KRIT_EDITOR_EDITOR

#define IMGUI_DEFINE_MATH_OPERATORS
#include "krit/Sprite.h"
#include "krit/render/Gl.h"
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

typedef std::function<float(RenderContext &)> MetricGetter;

struct MetricStats {
    float min, max, avg;
};

struct Metric {
    std::string name;
    MetricGetter getter;
    float buffer[60] = {0};
    int index = 0;
    bool wrapped = false;

    Metric(const std::string &_name, MetricGetter _getter)
        : name(_name), getter(_getter) {}

    void poll(RenderContext &ctx) {
        buffer[index++] = getter(ctx);
        if (index >= 60) {
            index %= 60;
            wrapped = true;
        }
    }

    MetricStats getStats() {
        float total = 0;
        float min = -1, max = -1;
        for (int i = 0; i < 60; ++i) {
            float v = buffer[i];
            total += v;
            if (min < 0 || v < min)
                min = v;
            if (max < 0 || v > max)
                max = v;
        }
        return MetricStats{.min = min, .max = max, .avg = total / 60};
    }
};

struct Overlay : public DevTool {
    static std::vector<Metric> metrics;

    static void addMetric(const std::string &name, MetricGetter getter) {
        metrics.emplace_back(name, getter);
    };

    float fpsBuffer[60] = {0};
    int index = 0;
    float elapsed = 0;

    void draw(krit::RenderContext &ctx) override;
};

struct Editor : public Sprite {
    static bool imguiInitialized;
    static GLuint imguiTextureId;
    // static SDL_Window *window;

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
