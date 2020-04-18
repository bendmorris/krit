#include "krit/input/InputContext.h"
#include "krit/sprites/Scene.h"

using namespace std;
using namespace krit;

namespace krit {

void Scene::update(UpdateContext &ctx) {
    if (this->fadingOut && this->fadeColor.a < this->maxAlpha) {
        this->fadeColor.a += ctx.elapsed / this->fadeDuration;
        if (this->fadeColor.a > this->maxAlpha) {
            this->fadeColor.a = this->maxAlpha;
        }
    } else if (!this->fadingOut && this->fadeColor.a > 0) {
        this->fadeColor.a -= ctx.elapsed / this->fadeDuration;
        if (this->fadeColor.a < 0) {
            this->fadeColor.a = 0;
        }
    }
    InputContext *oldInput = ctx.input;
    ctx.input = &this->input;
    this->input.update(ctx);
    this->layout.update(ctx);
    ctx.input = oldInput;
    this->mouseContext.update(ctx);
}

void Scene::render(RenderContext &ctx) {
    this->layout.render(ctx);
    if (this->fadeColor.a > 0) {
        DrawKey key;
        IntRectangle windowRect(0, 0, ctx.window->width(), ctx.window->height());
        Matrix m;
        ctx.drawCommandBuffer->addRect(
            key,
            windowRect,
            m,
            this->fadeColor.withAlpha(1 - pow(1 - this->fadeColor.a, 2))
        );
    }
}

}
