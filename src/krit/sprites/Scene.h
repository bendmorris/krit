#ifndef KRIT_SPRITES_SCENE
#define KRIT_SPRITES_SCENE

#include <string>

#include "krit/Sprite.h"
#include "krit/script/ScriptEngine.h"
#include "krit/utils/Color.h"
#include "quickjs.h"

namespace krit {
struct RenderContext;
struct ScriptEngine;
struct UpdateContext;

struct Scene : public Sprite {
    virtual void render(RenderContext &ctx) override;
    virtual void update(UpdateContext &ctx) override;

    Scene() {}

    void fadeOut(Color color, float fadeDuration = 0.5) {
        this->fadeColor = color;
        this->fadeDuration = fadeDuration / color.a;
        this->fadingOut = true;
        this->maxAlpha = color.a;
        this->fadeColor.a = 0;
    }

    void fadeIn(Color color, float fadeDuration = 0.5) {
        this->fadeColor = color;
        this->fadeDuration = fadeDuration / color.a;
        this->fadingOut = false;
        this->maxAlpha = color.a;
    }

private:
    Color fadeColor = 0;
    float maxAlpha = 0;
    float fadeDuration = 0;
    bool fadingOut = false;
};

struct ScriptScene : public Scene {
    ScriptEngine &engine;
    JSValue _update;
    JSValue _fixedUpdate;
    JSValue _render;
    JSValue _renderUi;

    ScriptScene(ScriptEngine &engine);
    ~ScriptScene();

    void update(UpdateContext &ctx) override;
    void fixedUpdate(UpdateContext &ctx) override;
    void render(RenderContext &ctx) override;

    private:
        float lastUpdateTime = 0;
        float lastFixedUpdateTime = 0;
        float lastRenderTime = 0;
};

}

#endif
