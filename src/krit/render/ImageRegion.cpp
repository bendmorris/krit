#include "ImageRegion.h"

#include "krit/Engine.h"

namespace krit {

ImageRegion ImageRegion::empty;

ImageRegion::ImageRegion(const std::string &id)
    : ImageRegion(engine->getImage(id)) {}
ImageRegion::ImageRegion(const std::string &id, const IntRectangle &rect)
    : ImageRegion(engine->getImage(id), rect) {}
ImageRegion::ImageRegion(const std::string &id, int x, int y, int w, int h)
    : ImageRegion(engine->getImage(id), x, y, w, h) {}

}
