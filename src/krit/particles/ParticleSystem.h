#ifndef KRIT_PARTICLES_PARTICLESYSTEM
#define KRIT_PARTICLES_PARTICLESYSTEM

#include "krit/particles/Interpolation.h"
#include "krit/particles/ParticleEffect.h"
#include <string>
#include <unordered_map>

namespace krit {

struct ParticleSystem {
    ParticleSystem();
    ParticleSystem(const std::string &path);

    ParticleEffect &addEffect(const std::string &name) {
        auto effect = new ParticleEffect(name);
        effects[name] = effect;
        return *effect;
    }

    std::unordered_map<std::string, ParticleEffect*> effects;
    std::unordered_map<std::string, InterpolationFunction*> functions;
};

}

#endif
