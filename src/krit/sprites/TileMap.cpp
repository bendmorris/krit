#include "krit/sprites/TileMap.h"

namespace krit {

void TileMap::render(RenderContext &ctx) {
    if (this->color.a <= 0) {
        return;
    }
    int tileWidth = this->properties.tileSize.width(),
        tileHeight = this->properties.tileSize.height();
    Dimensions scaledDimensions(tileWidth, tileHeight);
    ctx.transformDimensions(
        scaledDimensions.multiply(this->scale.x, this->scale.y)
    );
    Point scaledPosition = this->position;
    ctx.transformPoint(scaledPosition);
    int startX = max(0, static_cast<int>(floor(-scaledPosition.x / scaledDimensions.width()))),
        startY = max(0, static_cast<int>(floor(-scaledPosition.y / scaledDimensions.height()))),
        destX = min(static_cast<int>(startX + 1 + ceil(ctx.window->width() / scaledDimensions.width())), this->properties.sizeInTiles.width()),
        destY = min(static_cast<int>(startY + 1 + ceil(ctx.window->height() / scaledDimensions.height())), this->properties.sizeInTiles.height());

    DrawKey key;
    key.shader = this->shader;
    key.image = this->region.img;
    key.smooth = this->smooth;
    key.blend = this->blendMode;
    Matrix m(scaledDimensions.width() / tileWidth, 0, 0, scaledDimensions.height() / tileHeight, 0, 0);
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
                int tx = tile % this->tilemapSizeInTiles.width(),
                    ty = tile / this->tilemapSizeInTiles.width();
                m.tx = scaledPosition.x + scaledDimensions.width() * x;
                m.ty = scaledPosition.y + scaledDimensions.height() * y;
                IntRectangle rect(
                    this->region.rect.x + tx * (this->properties.fullTileWidth()) + this->properties.tilePadding.width(),
                    this->region.rect.y + ty * (this->properties.fullTileHeight()) + this->properties.tilePadding.height(),
                    tileWidth, tileHeight
                );
                ctx.addRectRaw(key, rect, m, this->color);
            }
        }
    }
}

}
