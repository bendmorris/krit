#ifndef KRIT_SPRITES_CANVAS
#define KRIT_SPRITES_CANVAS

#include "krit/Camera.h"
#include "krit/Sprite.h"
#include <memory>

namespace krit {

struct Canvas : public Sprite {
    Camera camera;
    std::unique_ptr<Sprite> child = nullptr;

    Canvas() {}

    void update(UpdateContext &ctx) override {
        this->camera.update(ctx);
        Camera *oldCamera = ctx.camera;
        ctx.camera = &this->camera;
        if (this->child) {
            this->child->update(ctx);
        }
        ctx.camera = oldCamera;
    }

    void render(RenderFunction &ctx) override {
        Camera *oldCamera = ctx.camera;
        ctx.camera = &this->camera;
        if (this->child) {
            this->child->render(ctx);
        }
        ctx.camera = oldCamera;
    }
};

}

#endif
