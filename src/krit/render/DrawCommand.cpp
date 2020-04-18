#include "krit/render/DrawCommand.h"

namespace krit {

DrawCall &DrawCommandBuffer::getDrawCall(DrawKey &key) {
    // TODO: fallthrough logic
    if (!this->commandTypes.empty() && this->commandTypes.back() == DrawTriangles) {
        DrawCall &last = this->get<DrawTriangles>().back();
        if (last.matches(key)) {
            return last;
        }
    }
    auto &call = this->emplace_back<DrawTriangles>();
    call.key = key;
    return call;
}

void DrawCommandBuffer::addRect(DrawKey &key, IntRectangle &rect, Matrix &matrix, Color color) {
    double uvx1;
    double uvy1;
    double uvx2;
    double uvy2;
    shared_ptr<ImageData> imageData = key.image;
    if (!imageData) {
        uvx1 = uvy1 = 0;
        uvx2 = rect.width;
        uvy2 = rect.height;
    } else {
        uvx1 = rect.x / static_cast<double>(imageData->width());
        uvy1 = rect.y / static_cast<double>(imageData->height());
        uvx2 = (rect.x + rect.width) / static_cast<double>(imageData->width());
        uvy2 = (rect.y + rect.height) / static_cast<double>(imageData->height());
    }

    // matrix transformations
    double xa = rect.width * matrix.a + matrix.tx;
    double yb = rect.width * matrix.b + matrix.ty;
    double xc = rect.height * matrix.c + matrix.tx;
    double yd = rect.height * matrix.d + matrix.ty;

    Triangle t(
        matrix.tx, matrix.ty,
        xa, yb,
        xc, yd
    );
    Triangle uv(
        uvx1, uvy1,
        uvx2, uvy1,
        uvx1, uvy2
    );

    DrawCall &call = this->getDrawCall(key);
    call.addTriangle(t, uv, color);

    Triangle t2(
        xc, yd,
        xa, yb,
        xa + rect.height * matrix.c, yb + rect.height * matrix.d
    );
    Triangle uv2(
        uvx1, uvy2,
        uvx2, uvy1,
        uvx2, uvy2
    );
    call.addTriangle(t2, uv2, color);
}

}
