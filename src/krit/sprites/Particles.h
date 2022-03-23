#ifndef KRIT_SPRITES_EMITTER
#define KRIT_SPRITES_EMITTER

#include "krit/Math.h"
#include "krit/Sprite.h"
#include "krit/asset/TextureAtlas.h"
#include "krit/render/BlendMode.h"
#include "krit/render/ImageRegion.h"
#include "krit/sprites/Image.h"
#include "krit/sprites/UserSprite.h"
#include "krit/utils/Color.h"
#include "krit/utils/Interpolation.h"
#include <algorithm>
#include <functional>
#include <list>
#include <map>
#include <string>
#include <unordered_map>

namespace krit {

typedef float (*InterpFunction)(float);

template <typename T> struct ParticleParam {
    T start;
    T end;
    InterpFunction lerp = nullptr;

    T eval(float t) { return start + (end - start) * (lerp ? lerp(t) : t); }
};

template <typename T> struct ParamRange {
    Range<T> start;
    Range<T> end;
    InterpFunction lerp = nullptr;
    bool relative = false;

    ParamRange() : start(0), end(0) {}
    ParamRange(T v) : start(v), end(v) {}
    ParamRange(T a, T b) : start(a), end(b) {}
    ParamRange(T a, T b, T c, T d) : start(a, b), end(c, d) {}

    void realize(ParticleParam<T> &param) {
        param.start = start.random();
        param.end = end.random();
        if (relative) {
            param.end = param.start + param.end;
        }
    }
};

struct ParticleType {
    int index;
    Image image;
    ParamRange<Color> color;
    ParamRange<float> alpha;
    ParamRange<float> scale;
    ParamRange<float> distance;
    ParamRange<float> angle;
    ParamRange<float> rotation;
    ParamRange<float> xOffset;
    ParamRange<float> yOffset;
    Range<float> duration;

    ParticleType(int id, ImageRegion region, BlendMode blend = Alpha);
};

struct Particle {
    float decay = 0;
    float duration;
    ParticleType *type;
    Point origin;
    ParticleParam<Color> color;
    ParticleParam<float> alpha;
    ParticleParam<float> scale;
    ParticleParam<float> distance;
    ParticleParam<float> angle;
    ParticleParam<float> rotation;
    ParticleParam<float> xOffset;
    ParticleParam<float> yOffset;

    Particle(ParticleType &type) : type(&type) {}
};

struct ParticleEmitter {
    ParticleType type;
    Point offset;
    int count;
    float start;
    float duration;

    int countAt(float t) {
        if (t <= start) {
            return 0;
        } else if (t >= start + duration) {
            return count;
        } else {
            return count * (t - start) / duration;
        }
    }
};

struct ParticleEffect {
    std::string name;
    float duration;
    std::vector<ParticleEmitter> emitters;
};

struct EffectInstance {
    ParticleEffect *effect;
    bool loop = false;
    float time = 0;
};

struct ParticleSystem : public VisibleSprite {
    ParticleSystem() {}

    std::size_t particleCount() { return _particles.size(); }

    void loadEffect(const std::string &);
    void emit(const std::string &, int count);
    void clear() {
        _effects.clear();
        _particles.clear();
    }

    void update(UpdateContext &ctx) override;
    void render(RenderContext &ctx) override;

private:
    std::vector<EffectInstance> _effects;
    std::vector<Particle> _particles;
    std::unordered_map<std::string, std::shared_ptr<ParticleEffect>> effects;
};

}

#endif
