#ifndef KRIT_RENDER_MATERIAL
#define KRIT_RENDER_MATERIAL

#include "krit/render/ImageRegion.h"
#include "krit/render/Shader.h"
#include "krit/render/Uniform.h"
#include <memory>
#include <vector>
#include <utility>

namespace krit {

struct Material {
    Shader *shader = nullptr;
    ImageData img;
    std::vector<std::pair<int, UniformValue>> uniforms;

    Material() {}
    Material(Shader *shader): shader(shader) {}

    void setUniform(const std::string &name, UniformValue value) {
        int loc = shader->getUniformLocation(name);
        if (loc > -1) {
            uniforms.push_back(std::make_pair(loc, value));
        }
    }

    void clear() {
        uniforms.clear();
    }
};

}

#endif
