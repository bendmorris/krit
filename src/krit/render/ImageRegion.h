#ifndef KRIT_RENDER_IMAGE_REGION
#define KRIT_RENDER_IMAGE_REGION

#include "krit/Math.h"
#include "krit/render/ImageData.h"
#include <memory>

namespace krit {

struct ImageRegion {
    static ImageRegion empty;

    static ImageRegion *create(std::shared_ptr<ImageData> img) {
        return new ImageRegion(img);
    }

    std::shared_ptr<ImageData> img;
    IntRectangle rect;
    std::string name;

    ImageRegion() : img(nullptr) {}
    ImageRegion(std::shared_ptr<ImageData> img, IntRectangle rect)
        : img(img), rect(rect) {}
    ImageRegion(std::shared_ptr<ImageData> img, int x, int y, int w, int h)
        : img(img), rect(x, y, w, h) {}
    ImageRegion(std::shared_ptr<ImageData> img)
        : img(img), rect(IntRectangle(0, 0, img->width(), img->height())) {}

    int &x() { return this->rect.x; }
    int &y() { return this->rect.y; }
    int &width() { return this->rect.width; }
    int &height() { return this->rect.height; }

    float uvx1() { return static_cast<float>(rect.x) / img->dimensions.x(); }
    float uvx2() {
        return (static_cast<float>(rect.x) + rect.width) / img->dimensions.x();
    }
    float uvy1() { return static_cast<float>(rect.y) / img->dimensions.y(); }
    float uvy2() {
        return (static_cast<float>(rect.y) + rect.height) / img->dimensions.y();
    }

    ImageRegion subRegion(int x, int y, int width, int height) {
        return ImageRegion(img, this->x() + x, this->y() + y, width, height);
    }
};

}

#endif
