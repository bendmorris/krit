#include "krit/sprites/NineSlice.h"

#include <memory>

#include "krit/Camera.h"
#include "krit/math/Matrix.h"
#include "krit/math/Rectangle.h"
#include "krit/render/DrawKey.h"
#include "krit/render/RenderContext.h"
#include "krit/render/SmoothingMode.h"

namespace krit {

NineSlice::NineSlice(ImageRegion &base, int leftWidth, int rightWidth,
                     int topHeight, int bottomHeight)
    : ul(base), uc(base), ur(base), cl(base), cc(base), cr(base), bl(base),
      bc(base), br(base) {
    smooth = SmoothLinear;

    int w = base.rect.width;
    int h = base.rect.height;
    int x0 = base.rect.x;
    int y0 = base.rect.y;
    int cx = x0 + leftWidth;
    int cw = w - leftWidth - rightWidth;
    int cy = y0 + topHeight;
    int ch = h - topHeight - bottomHeight;
    int rx = x0 + w - rightWidth;
    int by = y0 + h - bottomHeight;

    ul.rect.setTo(x0, y0, leftWidth, topHeight);
    uc.rect.setTo(cx, y0, cw, topHeight);
    ur.rect.setTo(rx, y0, rightWidth, topHeight);
    cl.rect.setTo(x0, cy, leftWidth, ch);
    cc.rect.setTo(cx, cy, cw, ch);
    cr.rect.setTo(rx, cy, rightWidth, ch);
    bl.rect.setTo(x0, by, leftWidth, bottomHeight);
    bc.rect.setTo(cx, by, cw, bottomHeight);
    br.rect.setTo(rx, by, rightWidth, bottomHeight);
}

void NineSlice::render(RenderContext &render) {
    float w = this->width();
    float h = this->height();

    float lw = this->ul.rect.width;
    float rw = this->ur.rect.width;
    float uh = this->ul.rect.height;
    float bh = this->bl.rect.height;

    if (scaleBorder) {
        lw *= render.camera->scale.x;
        rw *= render.camera->scale.x;
        bh *= render.camera->scale.y;
        uh *= render.camera->scale.y;
    }

    float cx = lw;
    float cw = w - lw - rw;
    float rx = w - rw;
    float cy = uh;
    float ch = h - uh - bh;
    float by = h - bh;

    DrawKey key;
    key.image = this->ul.img;
    key.smooth = this->smooth;
    key.blend = this->blendMode;

#define RENDER_SLICE(_r, _x, _y, _w, _h)                                       \
    do {                                                                       \
        Matrix m;                                                              \
        m.scale(_w / _r.width(), _h / _r.height())                             \
            .translate(_x - this->origin.x, _y - this->origin.y)               \
            .scale(this->scale.x, this->scale.y)                               \
            .translate(this->position.x, this->position.y);                    \
        render.addRect(key, _r.rect, m, this->color);                          \
    } while (0)

    RENDER_SLICE(this->ul, 0, 0, lw, uh);
    RENDER_SLICE(this->ur, rx, 0, rw, uh);
    RENDER_SLICE(this->bl, 0, by, lw, bh);
    RENDER_SLICE(this->br, rx, by, rw, bh);
    RENDER_SLICE(this->uc, cx, 0, cw, uh);
    RENDER_SLICE(this->bc, cx, by, cw, bh);
    RENDER_SLICE(this->cl, 0, cy, lw, ch);
    RENDER_SLICE(this->cr, rx, cy, rw, ch);
    RENDER_SLICE(this->cc, cx, cy, cw, ch);

#undef RENDER_SLICE
}

}
