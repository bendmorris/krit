#include "krit/sprites/Scene.h"
#include "krit/Engine.h"
#include "krit/input/InputContext.h"
#include "krit/render/DrawCommand.h"
#include "krit/script/ScriptClass.h"

namespace krit {

Scene::Scene(UpdateContext &ctx):
    asset(ctx.asset->cache), input(ctx.engine->controls) {}
Scene::Scene(UpdateContext &ctx, const std::string &layoutPath):
    asset(ctx.asset->cache), input(ctx.engine->controls), layout(layoutPath, *ctx.asset) {}

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
    ctx.asset = &asset;
    ctx.input = &input;
    this->input.update(ctx);
    this->layout.update(ctx);
    this->mouseContext.update(ctx);
}

void Scene::render(RenderContext &ctx) {
    Camera *oldCamera = ctx.camera;
    ctx.camera = &ctx.engine->uiCamera;
    this->layout.render(ctx);
    if (this->fadeColor.a > 0) {
        DrawKey key;
        IntRectangle windowRect(0, 0, ctx.window->width(), ctx.window->height());
        Matrix m;
        ctx.drawCommandBuffer->addRect(
            ctx,
            key,
            windowRect,
            m,
            this->fadeColor.withAlpha(1 - pow(1 - this->fadeColor.a, 2))
        );
    }
    ctx.camera = oldCamera;
}

void ScriptScene::update(UpdateContext &ctx) { engine.callVoid(_update); }
void ScriptScene::fixedUpdate(UpdateContext &ctx) { engine.callVoid(_fixedUpdate); }
void ScriptScene::render(RenderContext &ctx) { engine.callVoid(_render); }

}
