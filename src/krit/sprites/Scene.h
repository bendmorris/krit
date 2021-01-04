#ifndef KRIT_SPRITES_SCENE
#define KRIT_SPRITES_SCENE

#include "krit/asset/AssetContext.h"
#include "krit/sprites/Layout.h"
#include "krit/Sprite.h"
#include "krit/script/ScriptEngine.h"

namespace krit {

struct Scene: public Sprite {
    AssetContext asset;
    LayoutRoot layout;

    virtual void render(RenderContext &ctx) override;
    virtual void update(UpdateContext &ctx) override;

    Scene(UpdateContext &ctx);
    Scene(UpdateContext &ctx, const std::string &layoutPath);

    void fadeOut(Color color, double fadeDuration = 0.5) {
        this->fadeColor = color;
        this->fadeDuration = fadeDuration / color.a;
        this->fadingOut = true;
        this->maxAlpha = color.a;
        this->fadeColor.a = 0;
    }

    void fadeIn(Color color, double fadeDuration = 0.5) {
        this->fadeColor = color;
        this->fadeDuration = fadeDuration / color.a;
        this->fadingOut = false;
        this->maxAlpha = color.a;
    }

    private:
        Color fadeColor = 0;
        float maxAlpha = 0;
        double fadeDuration = 0;
        bool fadingOut = false;
};

struct ScriptScene: public Scene {
    ScriptEngine &engine;
    JSValue _update;
    JSValue _updateUi;
    JSValue _fixedUpdate;
    JSValue _render;
    JSValue _renderUi;

    ScriptScene(UpdateContext &ctx, ScriptEngine &engine);

    void update(UpdateContext &ctx) override;
    void fixedUpdate(UpdateContext &ctx) override;
    void render(RenderContext &ctx) override;
};

}

#endif
