#ifndef KRIT_MATH_ENTITY
#define KRIT_MATH_ENTITY

#include "krit/UpdateContext.h"
#include "krit/math/Dimensions.h"
#include "krit/math/Point.h"
#include "krit/render/BlendMode.h"
#include "krit/render/RenderContext.h"
#include "krit/render/SmoothingMode.h"

namespace krit {

struct SpriteShader;

struct Sprite {
    virtual void render(RenderContext &) {}
    virtual void update(UpdateContext &) {}
    virtual void fixedUpdate(UpdateContext &) {}

    virtual ~Sprite() = default;
};

struct SpriteStyle {
    Color color = Color::white();
    ScaleFactor scale;

    SpriteStyle(ScaleFactor scale, Color color = Color::white())
        : color(color), scale(scale) {}

    SpriteStyle(Color color = Color::white()) : color(color), scale() {}

    SpriteStyle lerp(const SpriteStyle &other, float mix) {
        return SpriteStyle(
            ScaleFactor(this->scale.x * (1 - mix) + other.scale.x * mix,
                        this->scale.y * (1 - mix) + other.scale.y * mix),
            this->color.lerp(other.color, mix));
    }
};

struct VisibleSprite : public Sprite, public SpriteStyle {
    Point position;
    Dimensions dimensions;
    SpriteShader *shader = nullptr;
    BlendMode blendMode = Alpha;
    SmoothingMode smooth = SmoothLinear;
    int zIndex = 0;

    VisibleSprite() {}
    virtual ~VisibleSprite() = default;

    float &width() { return this->dimensions.width(); }
    float &height() { return this->dimensions.height(); }

    virtual Point getPosition() { return this->position; }
    virtual Dimensions getSize() {
        return Dimensions(this->width() * this->scale.x,
                          this->height() * this->scale.y);
    }
    Rectangle getBounds() {
        auto p = getPosition();
        auto d = getSize();
        return Rectangle(p.x, p.y, d.x, d.y);
    }
    virtual void move(float x, float y) { this->position.setTo(x, y); }
    virtual void resize(float w, float h) {
        this->scale.setTo(w / this->width(), h / this->height());
    }

    void applyStyle(const SpriteStyle &style) {
        this->scale.setTo(style.scale.x, style.scale.y);
        this->color = style.color;
    }
};

}
#endif
