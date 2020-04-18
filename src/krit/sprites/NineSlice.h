#ifndef KRIT_SPRITES_NINESLICE
#define KRIT_SPRITES_NINESLICE

#include "krit/Sprite.h"
#include "krit/render/ImageRegion.h"
#include <memory>

using namespace std;
using namespace krit;

namespace krit {

struct NineSlice: public VisibleSprite {
    Point origin;

    ImageRegion ul;
    ImageRegion uc;
    ImageRegion ur;
    ImageRegion cl;
    ImageRegion cc;
    ImageRegion cr;
    ImageRegion bl;
    ImageRegion bc;
    ImageRegion br;

    NineSlice(ImageRegion &base, int border)
        : NineSlice(base, border, border) {}

    NineSlice(ImageRegion &base, int width, int height)
        : NineSlice(base, width, width, height, height) {}

    NineSlice(ImageRegion &base, int leftWidth, int rightWidth, int topHeight, int bottomHeight)
        : ul(base), uc(base), ur(base), cl(base), cc(base), cr(base), bl(base), bc(base), br(base)
    {
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

    void centerOrigin() {
        this->origin.setTo(this->width() / 2.0, this->height() / 2.0);
    }

    void resize(double w, double h) override { this->dimensions.setTo(w / this->scale.x, h / this->scale.y); }

    void render(RenderContext &ctx) override;
};

}

#endif
