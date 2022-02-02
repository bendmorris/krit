#include "krit/sprites/Scene.h"

#include <math.h>

#include "krit/App.h"
#include "krit/Engine.h"
#include "krit/UpdateContext.h"
#include "krit/math/Dimensions.h"
#include "krit/math/Matrix.h"
#include "krit/math/Rectangle.h"
#include "krit/render/DrawCommand.h"
#include "krit/render/DrawKey.h"
#include "krit/render/RenderContext.h"
#include "krit/script/ScriptEngine.h"
#if KRIT_ENABLE_TOOLS
#include "krit/editor/Editor.h"
#endif

namespace krit {
struct Camera;

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
}

void Scene::render(RenderContext &ctx) {
    Camera *oldCamera = ctx.camera;
    ctx.camera = &ctx.engine->uiCamera;
    if (this->fadeColor.a > 0) {
        DrawKey key;
        IntRectangle windowRect(0, 0, ctx.window->width(),
                                ctx.window->height());
        Matrix m;
        ctx.drawCommandBuffer->addRect(
            ctx, key, windowRect, m,
            this->fadeColor.withAlpha(1 - pow(1 - this->fadeColor.a, 2)));
    }
    ctx.camera = oldCamera;
}

ScriptScene::ScriptScene(ScriptEngine &engine)
    : Scene(), engine(engine),
      _update(JS_GetPropertyStr(engine.ctx, engine.exports, "update")),
      _fixedUpdate(
          JS_GetPropertyStr(engine.ctx, engine.exports, "fixedUpdate")),
      _render(JS_GetPropertyStr(engine.ctx, engine.exports, "render")),
      _renderUi(JS_GetPropertyStr(engine.ctx, engine.exports, "renderUi")) {

#if KRIT_ENABLE_TOOLS
    Overlay::addMetric("JS update time", [this](RenderContext &ctx) {
        return this->lastUpdateTime;
    });
    Overlay::addMetric("JS fupdate time", [this](RenderContext &ctx) {
        return this->lastFixedUpdateTime;
    });
    Overlay::addMetric("JS render time", [this](RenderContext &ctx) {
        return this->lastRenderTime;
    });
#endif
}

ScriptScene::~ScriptScene() {
    JS_FreeValue(engine.ctx, _update);
    JS_FreeValue(engine.ctx, _fixedUpdate);
    JS_FreeValue(engine.ctx, _render);
    JS_FreeValue(engine.ctx, _renderUi);
}

void ScriptScene::fixedUpdate(UpdateContext &ctx) {
    float start = ctx.app->time();
    engine.callVoid(_fixedUpdate, ctx);
    lastFixedUpdateTime = ctx.app->time() - start;
}
void ScriptScene::update(UpdateContext &ctx) {
    float start = ctx.app->time();
    engine.callVoid(_update, ctx);
    lastUpdateTime = ctx.app->time() - start;
}
void ScriptScene::render(RenderContext &ctx) {
    float start = ctx.app->time();
    engine.callVoid(_render, ctx);
    ctx.camera = &ctx.engine->uiCamera;
    engine.callVoid(_renderUi, ctx);
    ctx.camera = &ctx.engine->camera;
    lastRenderTime = ctx.app->time() - start;
}

}
