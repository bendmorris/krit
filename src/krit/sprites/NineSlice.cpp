#include "krit/sprites/NineSlice.h"

namespace krit {

#define RENDER_SLICE(_r, _x, _y, _w, _h) do {\
    Matrix m;\
    m.scale(_w / _r.width(), _h / _r.height())\
        .translate(_x - this->origin.x, _y - this->origin.y)\
        .scale(this->scale.x, this->scale.y)\
        .translate(this->position.x, this->position.y);\
    render.addRect(key, _r.rect, m, this->color);\
} while (0)

void NineSlice::render(RenderContext &render) {
    double w = this->width();
    double h = this->height();

    double lw = this->ul.rect.width;
    double rw = this->ur.rect.width;
    double uh = this->ul.rect.height;
    double bh = this->bl.rect.height;

    double cx = lw;
    double cw = w - lw - rw;
    double rx = w - rw;
    double cy = this->ul.rect.height;
    double ch = h - uh - bh;
    double by = h - bh;

    DrawKey key;
    key.image = this->ul.img;
    key.smooth = this->smooth;
    key.blend = this->blendMode;

    RENDER_SLICE(this->ul, 0, 0, lw, uh);
    RENDER_SLICE(this->ur, rx, 0, rw, uh);
    RENDER_SLICE(this->bl, 0, by, lw, bh);
    RENDER_SLICE(this->br, rx, by, rw, bh);
    RENDER_SLICE(this->uc, cx, 0, cw, uh);
    RENDER_SLICE(this->bc, cx, by, cw, bh);
    RENDER_SLICE(this->cl, 0, cy, lw, ch);
    RENDER_SLICE(this->cr, rx, cy, rw, ch);
    RENDER_SLICE(this->cc, cx, cy, cw, ch);
}

}
