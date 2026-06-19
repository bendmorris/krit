#pragma once

#include "krit/UpdateContext.h"
#include "krit/math/Dimensions.h"
#include "krit/math/Point.h"
#include "krit/render/BlendMode.h"
#include "krit/render/RenderContext.h"
#include "krit/render/SmoothingMode.h"

namespace krit {

struct SpriteShader;

struct Sprite {
    Point position;
    Dimensions dimensions;
    Color color = Color::white();
    SpriteShader *shader = nullptr;
    BlendMode blendMode = Alpha;
    SmoothingMode smooth = SmoothLinear;
    int zIndex = 0;

    virtual void render(RenderContext &) {}
    virtual void update() {}
    virtual void fixedUpdate() {}
    virtual ~Sprite() = default;
    virtual void move(float x, float y) { this->position.setTo(x, y); }
    virtual void resize(float w, float h) { this->dimensions.setTo(w, h); }

    float getWidth() { return dimensions.x; }
    void setWidth(float x) { dimensions.x = x; }
    float getHeight() { return dimensions.y; }
    void setHeight(float y) { dimensions.y = y; }
};

}
