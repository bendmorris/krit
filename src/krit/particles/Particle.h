#ifndef KRIT_PARTICLES_PARTICLE
#define KRIT_PARTICLES_PARTICLE

#include "krit/particles/ParticleEffect.h"
#include <string>
#include <unordered_map>

namespace krit {

struct Particle {
    ParticleEmission *emission;
    ResolvedProperties props;
    float elapsed = 0;

    Particle(ParticleEmission *emission);
};

}

#endif