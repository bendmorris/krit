#ifndef KRIT_PARTICLES_PARTICLEEFFECT
#define KRIT_PARTICLES_PARTICLEEFFECT

#include "krit/particles/ParticleProperties.h"
#include "krit/sprites/Image.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace krit {

struct ParticleEmission {
    float time = 0;
    int count = 1;
    float duration = 1;
    bool aligned = false;
    int layer = 0;
    std::string atlas;
    std::string region;
    BlendMode blend;
    UnresolvedProperties props;

    ParticleEmission() {}

    ParticleEmission &setStart(float v) { time = v; return *this; }
    ParticleEmission &setCount(int v) { count = v; return *this; }
    ParticleEmission &setDuration(float v) { duration = v; return *this; }
    ParticleEmission &align() { aligned = true; return *this; }
    ParticleEmission &setBlend(BlendMode v) { blend = v; return *this; }
    ParticleEmission &setLayer(int v) { layer = v; return *this; }
};

struct ParticleEffect {
    std::string name;
    // FIXME: should be set at emit time
    bool continuous = false;
    std::vector<ParticleEmission> timeline;

    ParticleEffect(const std::string &name): name(name) {}
    ParticleEffect() {}

    float duration() {
        float duration = 1.0f / 60;
        for (auto &item : timeline) {
            duration = std::max(duration, item.time + item.duration);
        }
        return duration;
    }

    ParticleEmission &addParticle() {
        timeline.emplace_back();
        return timeline.back();
    }
};

}

#endif