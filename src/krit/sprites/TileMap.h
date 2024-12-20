#ifndef KRIT_SPRITES_TILEMAP
#define KRIT_SPRITES_TILEMAP

#include <algorithm>
#include <cstdint>
#include <memory>
#include <vector>

#include "krit/Sprite.h"
#include "krit/math/Dimensions.h"
#include "krit/math/Rectangle.h"
#include "krit/render/ImageRegion.h"
#include "krit/sprites/NineSlice.h"

namespace krit {
struct ImageData;
struct RenderContext;

struct TileMapProperties {
    IntDimensions tileSize;
    IntDimensions sizeInTiles;
    IntDimensions tilePadding;

    int fullTileWidth() {
        return this->tileSize.x() + this->tilePadding.x() * 2;
    }

    int fullTileHeight() {
        return this->tileSize.y() + this->tilePadding.y() * 2;
    }

    TileMapProperties(int tileSizeX, int tileSizeY, int sizeInTilesX,
                      int sizeInTilesY, int tilePaddingX = 0,
                      int tilePaddingY = 0)
        : tileSize(tileSizeX, tileSizeY),
          sizeInTiles(sizeInTilesX, sizeInTilesY),
          tilePadding(tilePaddingX, tilePaddingY) {}
};

struct TileMap : public VisibleSprite {
    TileMapProperties properties;
    IntDimensions tilemapSizeInTiles;
    ImageRegion region;
    IntRectangle clip;

    TileMap(std::shared_ptr<ImageData> img, TileMapProperties properties)
        : properties(properties), region(img) {
        this->_init();
    }

    TileMap(ImageRegion &region, TileMapProperties properties)
        : properties(properties), region(region) {
        this->_init();
    }

    Dimensions getSize() override {
        return Dimensions(
            this->properties.sizeInTiles.x() * this->properties.tileSize.x(),
            this->properties.sizeInTiles.y() * this->properties.tileSize.y());
    }

    int16_t &getTile(int x, int y) {
        return this->tileData[y * this->properties.sizeInTiles.x() + x];
    }

    void setTile(int x, int y, int16_t value) { this->getTile(x, y) = value; }
    void clearTile(int x, int y) { this->getTile(x, y) = -1; }

    void render(RenderContext &ctx) override;

private:
    std::vector<int16_t> tileData;

    void _init() {
        int area =
            this->properties.sizeInTiles.x() * this->properties.sizeInTiles.y();
        this->tilemapSizeInTiles.setTo(
            this->region.x() / this->properties.fullTileWidth(),
            this->region.y() / this->properties.fullTileHeight());
        this->tileData.reserve(area);
        for (int i = 0; i < area; ++i) {
            this->tileData.push_back(-1);
        }
        this->clip.setTo(0, 0, this->properties.sizeInTiles.x(),
                         this->properties.sizeInTiles.y());
    }
};

}

#endif
