#include "krit/particles/Particle.h"

#include "krit/particles/ParticleEffect.h"

namespace krit {

Particle::Particle(ParticleEmission *emission) : emission(emission) {
    emission->props.resolve(props);
}

}
