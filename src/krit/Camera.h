#ifndef KRIT_CAMERA
#define KRIT_CAMERA

#include "krit/Math.h"
#include "krit/UpdateContext.h"

using namespace krit;

namespace krit {

class CameraTransform {
    public:
        Point scroll;
};

class Camera {
    public:
        enum ScaleMode {
            NoScale,
            Stretch,
            KeepWidth,
            KeepHeight,
        };

        Point position;
        Point offset;
        Point anchor;
        Dimensions dimensions;
        ScaleFactor scale;
        ScaleMode scaleMode = NoScale;

        union {
            struct {
                int min;
                int max;
            } minMax;
        } scaleData;

        double &width() { return this->dimensions.width(); }
        double &height() { return this->dimensions.height(); }

        Camera &center() {
            this->anchor.setTo(0.5, 0.5);
            return *this;
        }

        Camera &setLogicalSize(int width, int height) {
            this->dimensions.setTo(width, height);
            return *this;
        }

        Camera &noScale() {
            this->scaleMode = NoScale;
            return *this;
        }

        Camera &stretch() {
            this->scaleMode = Stretch;
            return *this;
        }

        Camera &keepWidth(int minHeight, int maxHeight) {
            this->scaleMode = KeepWidth;
            this->scaleData.minMax.min = minHeight;
            this->scaleData.minMax.max = maxHeight;
            return *this;
        }

        Camera &keepHeight(int minWidth, int maxWidth) {
            this->scaleMode = KeepHeight;
            this->scaleData.minMax.min = minWidth;
            this->scaleData.minMax.max = maxWidth;
            return *this;
        }

        Camera &move(double x, double y) {
            this->position.setTo(x, y);
            return *this;
        }

        Point &transformPoint(CameraTransform *transform, Point &p) {
            Point position = this->position;
            if (transform) {
                position.multiply(transform->scroll.x, transform->scroll.y);
            }
            return p.add(-position.x + (this->offset.x + this->anchor.x * this->dimensions.width()), -position.y + this->anchor.y * this->dimensions.height())
                .multiply(this->scale.x, this->scale.y);
        }

        Point &untransformPoint(CameraTransform *transform, Point &p) {
            Point position = this->position;
            if (transform) {
                position.multiply(transform->scroll.x, transform->scroll.y);
            }
            return p.multiply(1 / this->scale.x, 1 / this->scale.y)
                .add(position).add(-(this->offset.x + this->anchor.x * this->dimensions.width()), -this->anchor.y * this->dimensions.height());
        }

        Dimensions &scaleDimensions(CameraTransform *transform, Dimensions &d) {
            return d.setTo(d.width() * this->scale.x, d.height() * this->scale.y);
        }

        Dimensions &unscaleDimensions(CameraTransform *transform, Dimensions &d) {
            return d.setTo(d.width() / this->scale.x, d.height() / this->scale.y);
        }

        Matrix &transformMatrix(CameraTransform *transform, Matrix &m) {
            Point position = this->position;
            if (transform) {
                position.multiply(transform->scroll.x, transform->scroll.y);
            }
            return m.translate(-position.x + (this->offset.x + this->anchor.x * this->dimensions.width()), -position.y + this->anchor.y * this->dimensions.height())
                .scale(this->scale.x, this->scale.y);
        }

        Matrix &untransformMatrix(CameraTransform *transform, Matrix &m) {
            Point position = this->position;
            if (transform) {
                position.multiply(transform->scroll.x, transform->scroll.y);
            }
            return m.translate(1 / this->scale.x, 1 / this->scale.y)
                .scale(position.x - (this->offset.x + this->anchor.x * this->dimensions.width()), position.y - this->anchor.y * this->dimensions.height());
        }

        void update(UpdateContext &context);
};

}
#endif
