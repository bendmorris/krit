#ifndef KRIT_PARTICLES_PARTICLE
#define KRIT_PARTICLES_PARTICLE

#include "krit/particles/ParticleProperties.h"

namespace krit {

struct ParticleEmission;

struct Particle {
    ParticleEmission *emission;
    ResolvedProperties props;
    float elapsed = 0;

    Particle(ParticleEmission *emission);
};

}

#endif