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
    ParamRange(T a, T b, T c, T d, bool relative)
        : start(a, b), end(c, d), relative(relative) {}

    void realize(ParticleParam<T> &param) {
        param.start = start.random();
        param.end = end.random();
        if (relative) {
            param.end = param.start + param.end;
        }
        param.lerp = lerp;
    }
};

struct ParticleEmitter {
    ParamRange<Color> color;
    ParamRange<float> alpha;
    ParamRange<float> scale;
    ParamRange<float> distance;
    ParamRange<float> angle;
    ParamRange<float> rotation;
    ParamRange<float> xOffset;
    ParamRange<float> yOffset;
    Range<float> lifetime;
    std::string region;
    BlendMode blend = Alpha;
    bool aligned = true;
    int count = 1;
    float start = 0;
    float duration = 0;
    int zIndex = 0;

    std::unique_ptr<Image> image;

    ParticleEmitter();

    int countAt(float t) {
        if (t <= start) {
            return 0;
        } else if (t >= start + duration) {
            return count;
        } else {
            return 1 + (count - 1) * (t - start) / duration;
        }
    }
};

struct Particle {
    ParticleEmitter *emitter;
    ParticleParam<Color> color;
    ParticleParam<float> alpha;
    ParticleParam<float> scale;
    ParticleParam<float> distance;
    ParticleParam<float> angle;
    ParticleParam<float> rotation;
    ParticleParam<float> xOffset;
    ParticleParam<float> yOffset;
    float decay = 0;
    float duration;
    Point origin;

    Particle(ParticleEmitter &emitter) : emitter(&emitter) {}
};

struct ParticleEffect {
    static std::shared_ptr<ParticleEffect> load(const std::string &path);

    std::string name;
    std::vector<ParticleEmitter> emitters;
    float duration() {
        float dur = 0;
        for (auto &e : emitters) {
            if (e.start + e.duration > dur) {
                dur = e.start + e.duration;
            }
        }
        return dur;
    }
};

struct EffectInstance {
    std::shared_ptr<ParticleEffect> effect;
    Point origin;
    bool loop = false;
    float time = 0;
    float angle = 0;

    EffectInstance(std::shared_ptr<ParticleEffect> effect, const Point &at,
                   bool loop = false)
        : effect(effect), origin(at), loop(loop) {}
};

struct ParticleSystem : public VisibleSprite {
    ParticleSystem() {}
    ParticleSystem(const ParticleSystem &other) = default;

    std::function<void(Particle&, Image&)> transformer = nullptr;

    std::size_t particleCount() { return _particles.size(); }

    void loadAtlas(const std::string &path);
    void loadAtlas(std::shared_ptr<TextureAtlas> atlas) { this->atlas = atlas; }

    void loadEffect(const std::string &path);
    void registerEffect(std::shared_ptr<ParticleEffect> effect) {
        effects[effect->name] = effect;
        for (auto &emitter : effect->emitters) {
            emitter.image = std::unique_ptr<Image>(
                new Image(atlas->getRegion(emitter.region.c_str())));
            emitter.image->centerOrigin();
            emitter.image->blendMode = emitter.blend;
        }
    }
    EffectInstance &emit(const std::string &, const Point &at, bool loop = false);
    EffectInstance &emit(const std::string &id, float x, float y, bool loop = false) {
        return emit(id, Point(x, y), loop);
    }
    void clear() {
        _effects.clear();
        _particles.clear();
    }

    bool hasParticles() {
        return _particles.size() > 0;
    }

    void update(UpdateContext &ctx) override;
    void render(RenderContext &ctx) override;

private:
    std::shared_ptr<TextureAtlas> atlas;
    std::vector<EffectInstance> _effects;
    std::vector<Particle> _particles;
    std::unordered_map<std::string, std::shared_ptr<ParticleEffect>> effects;
};

}

#endif
