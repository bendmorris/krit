#include "krit/sprites/Particles.h"
#include "krit/App.h"
#include "krit/Math.h"
#include "krit/asset/AssetLoader.h"
#include "krit/io/Io.h"
#include "krit/utils/Panic.h"
#include "yaml.h"

namespace krit {

ParticleEmitter::ParticleEmitter()
    : color(Color::white()), alpha(1, 0), scale(1),
      distance(0, 100, 100, 200, true),
      angle(M_PI * -2, M_PI * 2, -M_PI, M_PI, true),
      rotation(M_PI * -2, M_PI * 2, -M_PI, M_PI, true), lifetime(1), count(5),
      start(0), duration(0) {}

EffectInstance &ParticleSystem::emit(const std::string &name, const Point &at, bool loop) {
    auto &effect = effects[name];
    _effects.emplace_back(effect, at, loop);
    return _effects.back();
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
        for (auto &emitter : instance.effect->emitters) {
            int oldParticles = emitter.countAt(oldTime);
            int curParticles = emitter.countAt(instance.time);
            for (int i = oldParticles; i < curParticles; ++i) {
                this->_particles.emplace_back(emitter);
                auto &particle = this->_particles.back();
                particle.origin = instance.origin;
                emitter.color.realize(particle.color);
                emitter.alpha.realize(particle.alpha);
                emitter.scale.realize(particle.scale);
                emitter.distance.realize(particle.distance);
                emitter.angle.realize(particle.angle);
                particle.angle.start += instance.angle;
                particle.angle.end += instance.angle;
                emitter.rotation.realize(particle.rotation);
                emitter.xOffset.realize(particle.xOffset);
                emitter.yOffset.realize(particle.yOffset);
                particle.duration = emitter.lifetime.random();
            }
        }
        if (instance.time >= instance.effect->duration()) {
            if (instance.loop) {
                instance.time = 0;
            } else {
                if (this->_effects.size() > 1) {
                    std::swap(this->_effects[i], this->_effects.back());
                }
                this->_effects.pop_back();
                --i;
            }
        }
        ++i;
    }
}

void ParticleSystem::render(RenderContext &ctx) {
    for (auto &particle : this->_particles) {
        auto &image = particle.emitter->image;
        float t = particle.decay;
        image->position = particle.origin;
        image->position += Vec3f(particle.xOffset.eval(t), particle.yOffset.eval(t));
        float distance = particle.distance.eval(t);
        float angle = particle.angle.eval(t);
        if (distance) {
            image->position += Vec3f(cos(angle) * distance, -sin(angle) * distance);
        }
        image->color = particle.color.eval(t);
        image->color.a = particle.alpha.eval(t);
        float scale = particle.scale.eval(t);
        image->scale = Vec2f(scale, scale);
        image->angle =
            particle.rotation.eval(t) - (particle.emitter->aligned ? angle : 0);
        image->zIndex = particle.emitter->zIndex;
        if (transformer) {
            transformer(particle, *image.get());
        }
        image->render(ctx);
    }
}

void ParticleSystem::loadAtlas(const std::string &path) {
    loadAtlasAsset(App::ctx.engine->getAtlas(path));
}

void ParticleSystem::loadEffect(const std::string &path) {
    registerEffect(App::ctx.engine->getParticle(path));
}

static char *yamlStr(yaml_node_t *node) {
    return static_cast<char *>(static_cast<void *>(node->data.scalar.value));
}

template <typename T>
static void parseValue(yaml_document_t *doc, yaml_node_t *node, T &val) {}
template <>
void parseValue(yaml_document_t *doc, yaml_node_t *node, float &val) {
    val = atof(yamlStr(node));
}
template <>
void parseValue(yaml_document_t *doc, yaml_node_t *node, Color &val) {
    val.r = atof(yamlStr(
        yaml_document_get_node(doc, node->data.sequence.items.start[0])));
    val.g = atof(yamlStr(
        yaml_document_get_node(doc, node->data.sequence.items.start[1])));
    val.b = atof(yamlStr(
        yaml_document_get_node(doc, node->data.sequence.items.start[2])));
}

template <typename T>
static void parseParam(yaml_document_t *doc, yaml_node_t *node,
                       ParamRange<T> &param) {
    for (yaml_node_pair_t *it = node->data.mapping.pairs.start;
         it < node->data.mapping.pairs.top; ++it) {
        yaml_node_t *keyNode = yaml_document_get_node(doc, it->key);
        yaml_node_t *valueNode = yaml_document_get_node(doc, it->value);
        const char *key = yamlStr(keyNode);
        if (!strcmp(key, "value")) {
            parseValue(doc,
                       yaml_document_get_node(
                           doc, valueNode->data.sequence.items.start[0]),
                       param.start.start);
            parseValue(doc,
                       yaml_document_get_node(
                           doc, valueNode->data.sequence.items.start[1]),
                       param.start.end);
            parseValue(doc,
                       yaml_document_get_node(
                           doc, valueNode->data.sequence.items.start[2]),
                       param.end.start);
            parseValue(doc,
                       yaml_document_get_node(
                           doc, valueNode->data.sequence.items.start[3]),
                       param.end.end);
        } else if (!strcmp(key, "ease")) {
            const char *ease = yamlStr(valueNode);
            param.lerp = interpolationFunctions[ease];
        } else if (!strcmp(key, "relative")) {
            param.relative = !strcmp(yamlStr(valueNode), "true");
        }
    }
}

std::shared_ptr<ParticleEffect> ParticleEffect::load(const std::string &path) {
    int len;
    char *manifest = app->io->read(path.c_str(), &len);
    yaml_parser_t parser;
    yaml_parser_initialize(&parser);
    yaml_parser_set_input_string(&parser, (unsigned char *)manifest, len);
    yaml_document_t doc;
    if (!yaml_parser_load(&parser, &doc)) {
        panic("failed to parse particle spec");
    }
    yaml_node_t *root = yaml_document_get_root_node(&doc);
    if (root->type != YAML_MAPPING_NODE) {
        panic("failed to parse particle spec");
    }

    std::shared_ptr<ParticleEffect> effect = std::make_shared<ParticleEffect>();

    for (yaml_node_pair_t *it = root->data.mapping.pairs.start;
         it < root->data.mapping.pairs.top; ++it) {
        yaml_node_t *keyNode = yaml_document_get_node(&doc, it->key);
        yaml_node_t *valueNode = yaml_document_get_node(&doc, it->value);
        const char *key = yamlStr(keyNode);
        if (!strcmp(key, "name")) {
            effect->name = yamlStr(valueNode);
        } else if (!strcmp(key, "emitters")) {
            for (yaml_node_item_t *it = valueNode->data.sequence.items.start;
                 it < valueNode->data.sequence.items.top; ++it) {
                yaml_node_t *node = yaml_document_get_node(&doc, *it);
                effect->emitters.emplace_back();
                auto &emitter = effect->emitters.back();
                for (yaml_node_pair_t *it = node->data.mapping.pairs.start;
                     it < node->data.mapping.pairs.top; ++it) {
                    yaml_node_t *keyNode =
                        yaml_document_get_node(&doc, it->key);
                    yaml_node_t *valueNode =
                        yaml_document_get_node(&doc, it->value);
                    const char *key = yamlStr(keyNode);
                    if (!strcmp(key, "region")) {
                        emitter.region = yamlStr(valueNode);
                    } else if (!strcmp(key, "blend")) {
                        const char *blend = yamlStr(valueNode);
                        if (!strcmp(blend, "add")) {
                            emitter.blend = Add;
                        } else if (!strcmp(blend, "subtract")) {
                            emitter.blend = Subtract;
                        } else if (!strcmp(blend, "multiply")) {
                            emitter.blend = Multiply;
                        } else if (!strcmp(blend, "screen")) {
                            emitter.blend = BlendScreen;
                        }
                    } else if (!strcmp(key, "aligned")) {
                        emitter.aligned = !strcmp(yamlStr(valueNode), "true");
                    } else if (!strcmp(key, "count")) {
                        emitter.count = atoi(yamlStr(valueNode));
                    } else if (!strcmp(key, "z")) {
                        emitter.zIndex = atoi(yamlStr(valueNode));
                    } else if (!strcmp(key, "start")) {
                        emitter.start = atof(yamlStr(valueNode));
                    } else if (!strcmp(key, "duration")) {
                        emitter.duration = atof(yamlStr(valueNode));
                    } else if (!strcmp(key, "lifetime")) {
                        emitter.lifetime.start =
                            atof(yamlStr(yaml_document_get_node(
                                &doc,
                                valueNode->data.sequence.items.start[0])));
                        emitter.lifetime.end =
                            atof(yamlStr(yaml_document_get_node(
                                &doc,
                                valueNode->data.sequence.items.start[1])));
                    } else if (!strcmp(key, "color")) {
                        parseParam(&doc, valueNode, emitter.color);
                    } else if (!strcmp(key, "alpha")) {
                        parseParam(&doc, valueNode, emitter.alpha);
                    } else if (!strcmp(key, "scale")) {
                        parseParam(&doc, valueNode, emitter.scale);
                    } else if (!strcmp(key, "distance")) {
                        parseParam(&doc, valueNode, emitter.distance);
                    } else if (!strcmp(key, "angle")) {
                        parseParam(&doc, valueNode, emitter.angle);
                    } else if (!strcmp(key, "rotation")) {
                        parseParam(&doc, valueNode, emitter.rotation);
                    } else if (!strcmp(key, "xOffset")) {
                        parseParam(&doc, valueNode, emitter.xOffset);
                    } else if (!strcmp(key, "yOffset")) {
                        parseParam(&doc, valueNode, emitter.yOffset);
                    }
                }
            }
        }
    }

    yaml_document_delete(&doc);
    yaml_parser_delete(&parser);
    app->io->free(manifest);

    return effect;
}

template <>
std::shared_ptr<ParticleEffect>
AssetLoader<ParticleEffect>::loadAsset(const std::string &path) {
    return ParticleEffect::load(path);
}

template <> size_t AssetLoader<ParticleEffect>::cost(ParticleEffect *effect) {
    return 1;
}

}
