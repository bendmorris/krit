#include "krit/sprites/TileMap.h"
#include "krit/Window.h"
#include "krit/math/Matrix.h"
#include "krit/math/Point.h"
#include "krit/render/DrawKey.h"
#include "krit/render/RenderContext.h"
#include "krit/utils/Color.h"
#include <math.h>

namespace krit {

void TileMap::render(RenderContext &ctx) {
    if (this->color.a <= 0) {
        return;
    }
    int tileWidth = this->properties.tileSize.x(),
        tileHeight = this->properties.tileSize.y();
    Dimensions scaledDimensions(tileWidth, tileHeight);
    scaledDimensions *= this->scale;
    // ctx.transformDimensions(scaledDimensions);
    Point pos = this->position;
    // ctx.transformPoint(pos);
    // FIXME
    int startX = 0, startY = 0, destX = properties.sizeInTiles.x(), destY = properties.sizeInTiles.y();
    // int startX = std::max(0, static_cast<int>(floor(-pos.x() /
    //                                                 scaledDimensions.x()))),
    //     startY = std::max(0, static_cast<int>(floor(-pos.y() /
    //                                                 scaledDimensions.y()))),
    //     destX = std::min(
    //         static_cast<int>(startX + 1 +
    //                          ceil(engine->window.x() / scaledDimensions.x())),
    //         this->properties.sizeInTiles.x()),
    //     destY = std::min(
    //         static_cast<int>(startY + 1 +
    //                          ceil(engine->window.y() / scaledDimensions.y())),
    //         this->properties.sizeInTiles.y());

    DrawKey key;
    key.shader = this->shader;
    key.image = this->region.img;
    key.smooth = this->smooth;
    key.blend = this->blendMode;
    Matrix4 m;
    m.identity();
    m.a() = scaledDimensions.x() / tileWidth;
    m.d() = scaledDimensions.y() / tileHeight;
    for (int y = startY; y < destY; ++y) {
        if (y < this->clip.y || y >= this->clip.bottom()) {
            continue;
        }
        for (int x = startX; x < destX; ++x) {
            if (x < this->clip.x || x >= this->clip.right()) {
                continue;
            }
            int tile = this->getTile(x, y);
            if (tile > -1) {
                int tx = tile % this->tilemapSizeInTiles.x(),
                    ty = tile / this->tilemapSizeInTiles.x();
                m.tx() = pos.x() + scaledDimensions.x() * x;
                m.ty() = pos.y() + scaledDimensions.y() * y;
                IntRectangle rect(this->region.rect.x +
                                      tx * (this->properties.fullTileWidth()) +
                                      this->properties.tilePadding.x(),
                                  this->region.rect.y +
                                      ty * (this->properties.fullTileHeight()) +
                                      this->properties.tilePadding.y(),
                                  tileWidth, tileHeight);
                Matrix4 m2(m);
                ctx.addRect(key, rect, m2, this->color);
            }
        }
    }
}

}
