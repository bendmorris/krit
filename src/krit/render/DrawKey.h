#ifndef KRIT_RENDER_DRAW_KEY
#define KRIT_RENDER_DRAW_KEY

#include "krit/Math.h"
#include "krit/render/BlendMode.h"
#include "krit/render/CommandBuffer.h"
#include "krit/render/FrameBuffer.h"
#include "krit/render/ImageData.h"
#include "krit/render/Shader.h"
#include "krit/render/SmoothingMode.h"
#include "krit/utils/Color.h"
#include <memory>
#include <tuple>
#include <vector>

namespace krit {

struct DrawKey {
    SpriteShader *shader = nullptr;
    std::shared_ptr<ImageData> image = nullptr;
    SmoothingMode smooth = SmoothMipmap;
    BlendMode blend = Alpha;

    DrawKey() {}

    bool operator==(const DrawKey &other) {
        return this->shader == other.shader && this->image == other.image &&
               this->smooth == other.smooth && this->blend == other.blend;
    }
};

}

#endif
