#include "krit/sprites/Particles.h"

namespace krit {

ParticleType::ParticleType(int index, ImageRegion region, BlendMode blend)
    : index(index), image(region), color(Color::white()), scale(1), angle(0) {
    image.blendMode = blend;
    image.centerOrigin();
}

void ParticleSystem::emit(const std::string &name, int count) {
    // TODO: load particle type
    // ParticleType &type = ...
    // std::uniform_real_distribution<float> r(0, 1);
    // for (int i = 0; i < count; ++i) {
    //     this->_particles.emplace_back(type);
    //     auto &particle = this->_particles.back();
    //     type.color.realize(particle.color);
    //     type.alpha.realize(particle.alpha);
    //     type.scale.realize(particle.scale);
    //     type.distance.realize(particle.distance);
    //     type.angle.realize(particle.angle);
    //     type.rotation.realize(particle.rotation);
    //     type.xOffset.realize(particle.xOffset);
    //     type.yOffset.realize(particle.yOffset);
    //     particle.duration = type.duration.random();
    // }
}

void ParticleSystem::update(UpdateContext &ctx) {
    size_t i = 0;
    while (i < this->_particles.size()) {
        auto &it = this->_particles[i];
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
    i = 0;
    while (i < this->_effects.size()) {
        auto &instance = this->_effects[i];
        float oldTime = instance.time;
        instance.time += ctx.elapsed;
        for (auto &it : instance.effect->emitters) {
            int oldParticles = it.countAt(oldTime);
            int curParticles = it.countAt(instance.time);
            for (int i = oldParticles; i < curParticles; ++i) {
                auto &type = it.type;
                this->_particles.emplace_back(type);
                auto &particle = this->_particles.back();
                type.color.realize(particle.color);
                type.alpha.realize(particle.alpha);
                type.scale.realize(particle.scale);
                type.distance.realize(particle.distance);
                type.angle.realize(particle.angle);
                type.rotation.realize(particle.rotation);
                type.xOffset.realize(particle.xOffset);
                type.yOffset.realize(particle.yOffset);
                particle.duration = type.duration.random();
            }
        }
        if (instance.time >= instance.effect->duration) {
            if (instance.loop) {
                instance.time = 0;
            } else {
                if (this->_effects.size() > 1) {
                    std::iter_swap(this->_effects.begin() + i,
                                   this->_effects.end() - 1);
                }
                this->_effects.pop_back();
            }
        }
    }
}

void ParticleSystem::render(RenderContext &ctx) {
    for (auto &particle : this->_particles) {
        auto &type = *particle.type;
        auto &image = type.image;
        float t = particle.decay;
        image.position.setTo(particle.origin);
        image.position.add(particle.xOffset.eval(t), particle.yOffset.eval(t));
        float distance = particle.distance.eval(t);
        if (distance) {
            float angle = particle.angle.eval(t);
            Point m(cos(angle) * distance, -sin(angle) * distance);
            image.position.add(m);
        }
        image.color = particle.color.eval(t);
        image.color.a = particle.alpha.eval(t);
        image.scale.setTo(particle.scale.eval(t));
        image.angle = particle.angle.eval(t);
        image.render(ctx);
    }
}

}
