#include "krit/sprites/Emitter.h"

namespace krit {

ParticleType::ParticleType(int index, ImageRegion region, BlendMode blend)
    : index(index), image(region), colorStart(Color::white(), Color::white()),
      colorEnd(Color::white(), Color::white()), scaleStart(1, 1),
      scaleEnd(1, 1), angle(0, M_PI * 2) {
    image.blendMode = blend;
    image.centerOrigin();
}

void Emitter::emit(const std::string &name, int count) {
    emit(system.get(name), count);
}

void Emitter::emit(ParticleType &type, int count) {
    std::uniform_real_distribution<float> r(0, 1);
    for (int i = 0; i < count; ++i) {
        this->_particles.emplace_back(type.index);
        auto &particle = this->_particles.back();
        particle.color.setTo(
            type.colorStart.start.lerp(type.colorStart.end, r(rng)),
            type.colorEnd.start.lerp(type.colorEnd.end, r(rng)));
        particle.scale.setTo(
            krit::lerp(type.scaleStart.start, type.scaleStart.end, r(rng)),
            krit::lerp(type.scaleEnd.start, type.scaleEnd.end, r(rng)));
        particle.rotation.setTo(
            krit::lerp(type.rotationStart.start, type.rotationStart.end,
                       r(rng)),
            krit::lerp(type.rotationEnd.start, type.rotationEnd.end, r(rng)));
        particle.start = this->position;
        particle.duration =
            krit::lerp(type.duration.start, type.duration.end, r(rng));
        particle.origin.setTo(r(rng) * 2 - 1, r(rng) * 2 - 1);
        particle.origin.normalize(krit::lerp(type.distanceStart.start,
                                             type.distanceStart.end, r(rng)));
        particle.origin.add(type.offset);
        particle.angle = krit::lerp(type.angle.start, type.angle.end, r(rng));
        particle.move.setTo(cos(particle.angle), -sin(particle.angle));
        particle.move.multiply(
            krit::lerp(type.distanceEnd.start, type.distanceEnd.end, r(rng)));
    }
}

void Emitter::update(UpdateContext &ctx) {
    size_t i = 0;
    int trailCount = 0;
    while (i < this->_particles.size()) {
        auto &it = this->_particles[i];
        auto &type = system.types[it.typeIndex];
        if (!it.isTrail && type.trail > 0) {
            ++trailCount;
        }
        ++i;
    }
    this->_particles.reserve(this->_particles.size() + trailCount);
    i = 0;
    while (i < this->_particles.size()) {
        auto &it = this->_particles[i];
        auto &type = system.types[it.typeIndex];
        if (!it.isTrail && type.trail > 0) {
            this->_particles.emplace_back(type.index);
            auto &trail = this->_particles.back();
            trail.duration = type.trail;
            trail.isTrail = true;
            trail.origin.setTo(this->position);
            trail.origin.add(it.origin);
            Color c = it.color.start.lerp(it.color.end, it.decay);
            c.a *= 0.5;
            trail.color.setTo(c);
            trail.scale.setTo(
                krit::lerp(it.scale.start, it.scale.end, it.decay));
            trail.angle = it.angle;
            Point m(it.move);
            // m.multiply(maybeLerp(it.decay, type.distanceLerp));
            m.multiply(it.decay);
            trail.origin.add(m);
        }
        it.decay += ctx.elapsed / it.duration;
        if (it.decay >= 1) {
            if (this->_particles.size() > 1) {
                std::iter_swap(this->_particles.begin() + i,
                               this->_particles.end() - 1);
            }
            this->_particles.pop_back();
        } else {
            ++i;
        }
    }
}

void Emitter::render(RenderContext &ctx) {
    for (auto &particle : this->_particles) {
        auto &type = system.types[particle.typeIndex];
        auto &image = type.image;
        if (particle.isTrail) {
            image.position.setTo(particle.origin);
            image.color = particle.color.start;
            image.scale.setTo(particle.scale.start);
        } else {
            image.position.setTo(particle.start);
            image.position.add(particle.origin);
            Point m(particle.move);
            m.multiply(maybeLerp(particle.decay, type.distanceLerp));
            image.position.add(m);
            image.color = particle.color.start.lerp(
                particle.color.end, maybeLerp(particle.decay, type.colorLerp));
            image.scale.setTo(
                krit::lerp(particle.scale.start, particle.scale.end,
                           maybeLerp(particle.decay, type.scaleLerp)));
        }
        image.angle = particle.angle +
                      krit::lerp(particle.rotation.start, particle.rotation.end,
                                 maybeLerp(particle.decay, type.rotationLerp));
        image.render(ctx);
    }
}

}
