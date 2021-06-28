#ifndef KRIT_RENDER_MATERIAL
#define KRIT_RENDER_MATERIAL

#include "krit/render/ImageRegion.h"
#include "krit/render/Shader.h"
#include "krit/render/Uniform.h"
#include <memory>
#include <utility>
#include <vector>

namespace krit {

struct Material {
    Shader *shader = nullptr;
    BlendMode blend = Alpha;
    ImageData img;
    std::vector<std::pair<std::string, UniformValue>> uniforms;
    std::unordered_map<std::string, int> uniformLocations;

    Material() {}
    Material(Shader *shader) : shader(shader) {}

    void setUniform(const std::string &name, UniformValue value) {
        uniforms.push_back(std::make_pair(name, value));
    }

    int findUniform(const std::string &name) {
        return shader->getUniformLocation(name);
    }

    void clear() { uniforms.clear(); }
};

}

#endif
