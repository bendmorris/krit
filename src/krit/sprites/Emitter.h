#ifndef KRIT_SPRITES_EMITTER
#define KRIT_SPRITES_EMITTER

#include "krit/render/BlendMode.h"
#include "krit/render/ImageRegion.h"
#include "krit/particles/Particle.h"
#include "krit/particles/ParticleSystem.h"
#include "krit/sprites/Image.h"
#include "krit/sprites/UserSprite.h"
#include "krit/utils/Color.h"
#include "krit/Sprite.h"
#include "krit/Math.h"
#include "krit/Utils.h"
#include <functional>
#include <list>
#include <map>
#include <string>
#include <unordered_map>

namespace krit {

struct EmissionTrack {
    ParticleEmission *data;
    Point position;
    Image image;
    float elapsed = 0;

    EmissionTrack(ParticleEmission *data, float elapsed = 0);
};

struct ParticleTrack {
    Image image;
    Particle particle;

    ParticleTrack(EmissionTrack &emission): image(emission.image), particle(emission.data) {}
};

struct EffectTrack {
    ParticleEffect *data;
    Point position;
    float elapsed = 0;
    bool continuous = false;

    EffectTrack(ParticleEffect *data): data(data) {}

    void stop();
};

struct Emitter: public VisibleSprite {
    ParticleSystem &system;

    Emitter(ParticleSystem &system): system(system) {}

    std::size_t particleCount() { return particles.size(); }

    EffectTrack &emit(const std::string &effectName);
    EffectTrack &emit(ParticleEffect *effect);
    // TODO: add a way to pass in ParticleProperties to modify particles on spawn

    Dimensions getSize() override { return Dimensions(0, 0); }
    void resize(float w, float h) override {}
    void clear();

    void update(UpdateContext &ctx) override;
    void render(RenderContext &ctx) override;

    private:
        std::vector<EmissionTrack> emissions;
        std::vector<EffectTrack> effects;
        std::vector<std::vector<ParticleTrack>> particles;
};

}

#endif
