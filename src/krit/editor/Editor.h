#ifndef KRIT_EDITOR_EDITOR
#define KRIT_EDITOR_EDITOR

#include "imgui.h"
#include "krit/Sprite.h"

namespace krit {

struct DevTool {
    virtual void draw(krit::RenderContext &ctx) {}
};

struct Overlay: public DevTool {
    int fpsBuffer[4] = {0};
    int index = 0;

    void draw(krit::RenderContext &ctx) override;
};

struct Editor: public Sprite {
    constexpr static const double SCALE = 1.5;

    static bool imguiInitialized;
    static GLuint imguiTextureId;
    static SDL_Window *window;

    ImGuiIO *io;
    std::shared_ptr<ImageData> texture;

    Editor();

    void addDevTool(DevTool *tool) {
        devTools.emplace_back(tool);
    }

    void update(krit::UpdateContext&) override;
    void render(krit::RenderContext&) override;

    private:
        std::vector<std::unique_ptr<DevTool>> devTools;
};

}

#endif
