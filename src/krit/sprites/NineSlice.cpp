#include "krit/sprites/NineSlice.h"

#include <memory>

#include "krit/Camera.h"
#include "krit/math/Matrix.h"
#include "krit/math/Rectangle.h"
#include "krit/render/DrawKey.h"
#include "krit/render/RenderContext.h"
#include "krit/render/SmoothingMode.h"

namespace krit {

void NineSlice::render(RenderContext &render) {
    if (color.a <= 0 && !shader) {
        return;
    }

    IntRectangle ul, uc, ur, cl, cc, cr, bl, bc, br;
    {
        int basew = base.rect.width;
        int baseh = base.rect.height;
        int x0 = base.rect.x;
        int y0 = base.rect.y;
        int cx = x0 + leftWidth;
        int cw = basew - leftWidth - rightWidth;
        int cy = y0 + topHeight;
        int ch = baseh - topHeight - bottomHeight;
        int rx = x0 + basew - rightWidth;
        int by = y0 + baseh - bottomHeight;
        ul.setTo(x0, y0, leftWidth, topHeight);
        uc.setTo(cx, y0, cw, topHeight);
        ur.setTo(rx, y0, rightWidth, topHeight);
        cl.setTo(x0, cy, leftWidth, ch);
        cc.setTo(cx, cy, cw, ch);
        cr.setTo(rx, cy, rightWidth, ch);
        bl.setTo(x0, by, leftWidth, bottomHeight);
        bc.setTo(cx, by, cw, bottomHeight);
        br.setTo(rx, by, rightWidth, bottomHeight);
    }

    float w = this->dimensions.x;
    float h = this->dimensions.y;
    float lw = ul.width * borderScale;
    float rw = ur.width * borderScale;
    float uh = ul.height * borderScale;
    float bh = bl.height * borderScale;
    float cx = lw;
    float cw = w - lw - rw;
    float rx = w - rw;
    float cy = uh;
    float ch = h - uh - bh;
    float by = h - bh;

    DrawKey key;
    key.image = this->base.img;
    key.smooth = this->smooth;
    key.blend = this->blendMode;
    key.shader = this->shader;

    auto renderSlice = [&](const IntRectangle &_r, float _x, float _y, float _w,
                           float _h) {
        if (w > 0 && h > 0 && _r.width > 0 && _r.height > 0) {
            Matrix4 m;
            m.identity();
            m.scale(_w / _r.width, _h / _r.height);
            m.translate(_x - this->origin.x, _y - this->origin.y);
            if (pitch) {
                m.pitch(pitch);
            }
            // m.scale(this->scale.x, this->scale.y);
            m.translate(this->position.x, this->position.y, this->position.z);
            render.addRect(key, _r, m, this->color);
        }
    };

    renderSlice(ul, 0, 0, lw, uh);
    renderSlice(ur, rx, 0, rw, uh);
    renderSlice(bl, 0, by, lw, bh);
    renderSlice(br, rx, by, rw, bh);
    renderSlice(uc, cx, 0, cw, uh);
    renderSlice(bc, cx, by, cw, bh);
    renderSlice(cl, 0, cy, lw, ch);
    renderSlice(cr, rx, cy, rw, ch);
    renderSlice(cc, cx, cy, cw, ch);
}

}
