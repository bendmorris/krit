#include "krit/sprites/Scene.h"
#include "krit/Engine.h"
#include "krit/AssetId.h"
#include "krit/input/InputContext.h"
#include "krit/render/DrawCommand.h"
#include "krit/script/ScriptClass.h"
#include "krit/script/ScriptValue.h"
#include "krit/render/RenderContext.h"

namespace krit {

Scene::Scene(UpdateContext &ctx):
    asset(ctx.asset->cache) {}
Scene::Scene(UpdateContext &ctx, const std::string &layoutPath):
    asset(ctx.asset->cache), layout(layoutPath, *ctx.asset) {}

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
    this->layout.update(ctx);
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

ScriptScene::ScriptScene(UpdateContext &ctx, ScriptEngine &engine):
    Scene(ctx),
    engine(engine),
    _update(JS_GetPropertyStr(engine.ctx, engine.exports, "update")),
    _updateUi(JS_GetPropertyStr(engine.ctx, engine.exports, "updateUi")),
    _fixedUpdate(JS_GetPropertyStr(engine.ctx, engine.exports, "fixedUpdate")),
    _render(JS_GetPropertyStr(engine.ctx, engine.exports, "render")),
    _renderUi(JS_GetPropertyStr(engine.ctx, engine.exports, "renderUi"))
{
}

void ScriptScene::fixedUpdate(UpdateContext &ctx) { engine.callVoid(_fixedUpdate, ctx); }
void ScriptScene::update(UpdateContext &ctx) {
    engine.callVoid(_update, ctx);
    ctx.camera = &ctx.engine->uiCamera;
    engine.callVoid(_updateUi, ctx);
    ctx.camera = &ctx.engine->camera;
}
void ScriptScene::render(RenderContext &ctx) {
    engine.callVoid(_render, ctx);
    ctx.camera = &ctx.engine->uiCamera;
    engine.callVoid(_renderUi, ctx);
    ctx.camera = &ctx.engine->camera;
}

}
