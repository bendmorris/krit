#ifndef KRIT_SPRITES_BUTTON
#define KRIT_SPRITES_BUTTON

#include "krit/Sprite.h"
#include "krit/sprites/BitmapText.h"
#include "krit/sprites/NineSlice.h"
#include "krit/render/ImageRegion.h"
#include <memory>

namespace krit {

struct Button: public NineSlice {
    BitmapText label;
    SpriteStyle defaultStyle;
    SpriteStyle focusedStyle;
    SpriteStyle pressedStyle;

    bool focused = false;
    bool pressed = false;

    SpriteStyle previousStyle;
    double transitionTime = 0.15;
    double transition = 0;

    Button(ImageRegion &base, int border, BitmapTextOptions &labelOptions, const std::string &labelText)
        : Button(base, border, border, border, border, labelOptions, labelText) {}

    Button(ImageRegion &base, int borderWidth, int borderHeight, BitmapTextOptions &labelOptions, const std::string &labelText)
        : Button(base, borderWidth, borderWidth, borderHeight, borderHeight, labelOptions, labelText) {}

    Button(ImageRegion &base, int lw, int rw, int th, int bh, BitmapTextOptions &labelOptions, const std::string &labelText)
        : NineSlice(base, lw, rw, th, bh), label(labelOptions)
    {
        label.setText(labelText);
    }

    Dimensions getSize() override { return Dimensions(this->width(), this->height()); }

    void resize(double w, double h) override {
        this->dimensions.setTo(w, h);
        this->label.refresh();
    }

    void setState(bool focused, bool pressed) {
        if (this->focused != focused || this->pressed != pressed) {
            this->transition = 0;
            this->previousStyle = *this;
        }
        this->focused = focused;
        this->pressed = pressed;
    }

    void update(UpdateContext &ctx) override {
        if (this->transition < 1) {
            this->transition += ctx.elapsed / this->transitionTime;
            if (this->transition > 1) this->transition = 1;
        }
        SpriteStyle &newStyle = this->pressed ? this->pressedStyle : (this->focused ? this->focusedStyle : this->defaultStyle);
        if (transition >= 1) {
            this->applyStyle(newStyle);
        } else {
            SpriteStyle lerpStyle = this->previousStyle.lerp(newStyle, this->transition);
            this->applyStyle(lerpStyle);
        }
        this->label.scale.copyFrom(this->scale);
        this->label.update(ctx);
    }

    void render(RenderContext &ctx) override {
        Point position = this->position;
        NineSlice::centerOrigin();
        this->position.add(this->origin);
        NineSlice::render(ctx);
        this->position = position;
        Dimensions labelSize = this->label.getSize();
        this->label.position.setTo(
            this->position.x + (this->width() - labelSize.width() * this->label.scale.x) / 2,
            this->position.y + (this->height() - labelSize.height() * this->label.scale.y) / 2
        );
        this->label.render(ctx);
    }
};

}

#endif
