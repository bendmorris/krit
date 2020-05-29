#include "krit/Engine.h"
#include "krit/render/DrawCommand.h"

namespace krit {

void Engine::render(RenderContext &ctx) {
    ctx.camera = &this->camera;
    ctx.userData = this->userData;

    invoke(this->onRender, &ctx);

    if (this->bgColor.a > 0) {
        DrawKey key;
        IntRectangle windowRect(0, 0, ctx.window->width(), ctx.window->height());
        Matrix m;
        ctx.drawCommandBuffer->addRect(
            ctx,
            key,
            windowRect,
            m,
            this->bgColor
        );
    }
    if (this->root) this->root->render(ctx);
    if (this->editor) this->editor->render(ctx);

    invoke(this->postRender, &ctx);
}

}