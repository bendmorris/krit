#include "krit/sprites/Emitter.h"
#include <cmath>

namespace krit {

EmissionTrack::EmissionTrack(UpdateContext &ctx, ParticleEmission *data, float elapsed):
    data(data), elapsed(elapsed),
    image(data->atlas.empty() ? ctx.asset->getImage(data->region) : ctx.asset->getTextureAtlas(data->atlas)->getRegion(data->region))
{
    image.blendMode = data->blend;
    image.centerOrigin();
}

void EffectTrack::stop() {
    continuous = false;
    elapsed = data->duration();
}

EffectTrack &Emitter::emit(const std::string &effectType) {
    effects.emplace_back(system.effects[effectType]);
    // FIXME
    return effects.back();
}

EffectTrack & Emitter::emit(ParticleEffect *effect) {
    effects.emplace_back(effect);
    // FIXME
    return effects.back();
}

void Emitter::clear() {
    effects.clear();
}

static int pcount(float start, float duration, int count, float t) {
    if (t < start) return 0;
    else if (t >= start + duration) return count;
    else return count * (t - start) / duration;
}

void Emitter::update(UpdateContext &ctx) {
    // update particles
    for (auto &particles : this->particles) {
        for (size_t i = 0; i < particles.size(); ++i) {
            auto &particle = particles[i].particle;
            particle.elapsed += ctx.elapsed / particle.props.duration;
            if (particle.elapsed > 1) {
                if (particles.size() > 1) {
                    std::iter_swap(particles.begin() + i, particles.end() - 1);
                }
                particles.pop_back();
                --i;
            }
        }
    }

    // update emissions to generate new particles
    for (auto &emission : emissions) {
        float elapsed = ctx.elapsed;
        if (emission.elapsed < 0) {
            elapsed -= emission.elapsed;
            emission.elapsed = 0;
        }
        int last = pcount(emission.data->time, emission.data->duration, emission.data->count, emission.elapsed);
        emission.elapsed += elapsed;
        int current = pcount(emission.data->time, emission.data->duration, emission.data->count, emission.elapsed);
        for (int i = 0; i < current - last; ++i) {
            if (particles.size() < emission.data->layer + 1) {
                particles.resize(emission.data->layer + 1);
            }
            auto &particles = this->particles[emission.data->layer];
            particles.emplace_back(emission);
            particles.back().particle.props.originX += emission.position.x;
            particles.back().particle.props.originY += emission.position.y;
        }
    }
    for (size_t i = 0; i < emissions.size(); ++i) {
        auto &emission = emissions[i];
        if (emission.elapsed >= emission.data->duration) {
            if (emissions.size() > 1 && i < emissions.size() - 1) {
                std::iter_swap(emissions.begin() + i, emissions.end() - 1);
            }
            emissions.pop_back();
        }
    }

    // update effects to generate new emissions
    for (auto &effect : effects) {
        float duration = effect.data->duration();
        float elapsed = ctx.elapsed;
        float last = effect.elapsed;
        effect.elapsed += ctx.elapsed;
        
        for (auto &emission : effect.data->timeline) {
            if ((!last || emission.time < last) && emission.time < effect.elapsed) {
                emissions.emplace_back(ctx, &emission, emission.time - effect.elapsed);
                emissions.back().position.setTo(effect.position);
            }
        }
    }
    for (int i = 0; i < effects.size(); ++i) {
        auto &effect = effects[i];
        if (effect.elapsed > effect.data->duration()) {
            if (effect.continuous) {
                effect.elapsed = 0;
            } else {
                if (effects.size() > 1 && i < effects.size() - 1) {
                    std::iter_swap(effects.begin() + i, effects.end() - 1);
                }
                effects.pop_back();
            }
        }
    }
}

void Emitter::render(RenderContext &ctx) {
    for (auto &particles : this->particles) {
        for (auto &track : particles) {
            auto &particle = track.particle;
            auto &image = track.image;
            auto &props = particle.props;
            auto &emission = *particle.emission;
            float t = particle.elapsed / particle.props.duration;
            image.position.setTo(position.x + props.originX, position.y + props.originY);
            float distance = props.evaluate(props.distance, t),
                orthoDistance = props.evaluate(props.orthoDistance, t);
            float s = std::sin(props.direction), c = std::cos(props.direction);
            image.position.add(c * distance, -s * distance);
            image.position.add(s * orthoDistance, c * orthoDistance);
            image.angle = (emission.aligned ? props.direction : 0) + props.evaluate(props.rotation, t);
            image.color = Color(props.evaluate(props.red, t), props.evaluate(props.green, t), props.evaluate(props.blue, t), props.evaluate(props.alpha, t));
            float scale = props.evaluate(props.scale, t);
            image.scale.setTo(scale, scale);
            image.render(ctx);
        }
    }
}

}
