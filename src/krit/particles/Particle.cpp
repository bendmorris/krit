#include "krit/particles/Particle.h"

namespace krit {

Particle::Particle(ParticleEmission *emission): emission(emission) {
    emission->props.resolve(props);
}

}
