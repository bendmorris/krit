#include "krit/Camera.h"

namespace krit {

void Camera::update(UpdateContext &context) {
    switch (this->scaleMode) {
        case NoScale: {
            // nothing to do
            break;
        }
        case Stretch: {
            // stretch to show the camera's logical size
            this->scale.setTo(
                context.window->width() / this->dimensions.width(),
                context.window->height() / this->dimensions.height()
            );
            break;
        }
        case KeepWidth: {
            int min = this->scaleData.minMax.min,
                max = this->scaleData.minMax.max;
            int visibleHeight = context.window->height() * this->dimensions.width() / context.window->width();
            if (visibleHeight < min) {
                visibleHeight = min;
            } else if (visibleHeight > max) {
                visibleHeight = max;
            }
            this->scale.setTo(
                static_cast<double>(context.window->width()) / this->dimensions.width(),
                static_cast<double>(context.window->height()) / visibleHeight
            );
            this->offset.y = (context.window->height() / this->scale.y - this->dimensions.height()) / 2;
            break;
        }
        case KeepHeight: {
            int min = this->scaleData.minMax.min,
                max = this->scaleData.minMax.max;
            int visibleWidth = context.window->width() * this->dimensions.height() / context.window->height();
            if (visibleWidth < min) {
                visibleWidth = min;
            } else if (visibleWidth > max) {
                visibleWidth = max;
            }
            this->scale.setTo(
                static_cast<double>(context.window->width()) / visibleWidth,
                static_cast<double>(context.window->height()) / this->dimensions.height()
            );
            this->offset.x = (context.window->width() / this->scale.x - this->dimensions.width()) / 2;
            break;
        }
    }
}

}
