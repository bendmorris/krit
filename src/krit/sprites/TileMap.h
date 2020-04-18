#ifndef KRIT_SPRITES_TILEMAP
#define KRIT_SPRITES_TILEMAP

#include "krit/Sprite.h"
#include "krit/sprites/BitmapText.h"
#include "krit/sprites/NineSlice.h"
#include "krit/render/ImageRegion.h"
#include <memory>

using namespace std;
using namespace krit;

namespace krit {

struct TileMapProperties {
    IntDimensions tileSize;
    IntDimensions sizeInTiles;
    IntDimensions tilePadding;

    int fullTileWidth() {
        return this->tileSize.width() + this->tilePadding.width() * 2;
    }

    int fullTileHeight() {
        return this->tileSize.height() + this->tilePadding.height() * 2;
    }

    TileMapProperties(int tileSizeX, int tileSizeY, int sizeInTilesX, int sizeInTilesY, int tilePaddingX = 0, int tilePaddingY = 0)
        : tileSize(tileSizeX, tileSizeY), sizeInTiles(sizeInTilesX, sizeInTilesY), tilePadding(tilePaddingX, tilePaddingY) {}
};

struct TileMap: public VisibleSprite {
    TileMapProperties properties;
    IntDimensions tilemapSizeInTiles;
    ImageRegion region;
    IntRectangle clip;

    TileMap(shared_ptr<ImageData> img, TileMapProperties properties)
        : region(img), properties(properties)
    {
        this->_init();
    }

    TileMap(ImageRegion &region, TileMapProperties properties)
        : region(region), properties(properties)
    {
        this->_init();
    }

    Dimensions getSize() override {
        return Dimensions(
            this->properties.sizeInTiles.width() * this->properties.tileSize.width(),
            this->properties.sizeInTiles.height() * this->properties.tileSize.height()
        );
    }

    int16_t &getTile(int x, int y) { return this->tileData[y * this->properties.sizeInTiles.width() + x]; }

    void setTile(int x, int y, int16_t value) { this->getTile(x, y) = value; }
    void clearTile(int x, int y) { this->getTile(x, y) = -1; }

    // void update(UpdateContext &ctx) override;
    void render(RenderContext &ctx) override;

    private:
        vector<int16_t> tileData;

        void _init() {
            this->tilemapSizeInTiles.setTo(
                this->region.width() / this->properties.fullTileWidth(),
                this->region.height() / this->properties.fullTileHeight()
            );
            this->tileData.reserve(this->properties.sizeInTiles.area());
            for (int i = 0; i < this->properties.sizeInTiles.area(); ++i) {
                this->tileData.push_back(-1);
            }
            this->clip.setTo(
                0, 0,
                this->properties.sizeInTiles.width(),
                this->properties.sizeInTiles.height()
            );
        }
};

}

#endif
