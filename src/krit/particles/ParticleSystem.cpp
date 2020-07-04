#include "krit/particles/ParticleSystem.h"
#include "krit/io/Io.h"
#include "yaml.h"
#include <cassert>

namespace krit {

ParticleSystem::ParticleSystem() {
    functions["linear"] = &InterpolationFunction::linearInterpolation;
}

static char *yamlStringToCString(yaml_char_t *s) {
    return static_cast<char*>(static_cast<void*>(s));
}

ParticleSystem::ParticleSystem(const std::string &path) {
    functions["linear"] = &InterpolationFunction::linearInterpolation;

    int length;
    const unsigned char *contents = static_cast<unsigned char*>(static_cast<void*>(IoRead::read(path, &length)));

    yaml_parser_t parser;
    yaml_parser_initialize(&parser);
    yaml_parser_set_input_string(&parser, contents, length);
    yaml_document_t doc;
    if (!yaml_parser_load(&parser, &doc)) {
        panic("failed to parse particle system YAML: %s", path.c_str());
    }

    yaml_node_t *root = yaml_document_get_root_node(&doc);
    assert(root->type == YAML_MAPPING_NODE);
    for (yaml_node_pair_t *it = root->data.mapping.pairs.start; it < root->data.mapping.pairs.top; ++it) {
        yaml_node_t *keyNode = yaml_document_get_node(&doc, it->key);
        char *keyName = yamlStringToCString(keyNode->data.scalar.value);
        yaml_node_t *value = yaml_document_get_node(&doc, it->value);
        if (!strcmp(keyName, "functions")) {
            // parse functions
            // TODO
        } else if (!strcmp(keyName, "effects")) {
            // parse effects
            assert(value->type == YAML_MAPPING_NODE);
            for (yaml_node_pair_t *it = value->data.mapping.pairs.start; it < value->data.mapping.pairs.top; ++it) {
                auto effect = new ParticleEffect();
                yaml_node_t *keyNode = yaml_document_get_node(&doc, it->key);
                char *keyName = yamlStringToCString(keyNode->data.scalar.value);
                effect->name = keyName;
                yaml_node_t *value = yaml_document_get_node(&doc, it->value);
                assert(value->type == YAML_SEQUENCE_NODE);
                for (yaml_node_item_t *childIndex = value->data.sequence.items.start; childIndex < value->data.sequence.items.top; ++childIndex) {
                    yaml_node_t *child = yaml_document_get_node(&doc, *childIndex);
                    auto &particle = effect->addParticle();
                    assert(child->type == YAML_MAPPING_NODE);
                    for (yaml_node_pair_t *it = child->data.mapping.pairs.start; it < child->data.mapping.pairs.top; ++it) {
                        yaml_node_t *keyNode = yaml_document_get_node(&doc, it->key);
                        char *keyName = yamlStringToCString(keyNode->data.scalar.value);
                        yaml_node_t *value = yaml_document_get_node(&doc, it->value);
                        if (!strcmp(keyName, "atlas")) {
                            particle.atlas = yamlStringToCString(value->data.scalar.value);
                        } else if (!strcmp(keyName, "region")) {
                            particle.region = yamlStringToCString(value->data.scalar.value);
                        } else if (!strcmp(keyName, "blend")) {
                            auto blend = yamlStringToCString(value->data.scalar.value);
                            if (!strcmp(blend, "alpha")) particle.blend = Alpha;
                            else if (!strcmp(blend, "add")) particle.blend = Add;
                            else if (!strcmp(blend, "subtract")) particle.blend = Subtract;
                            else if (!strcmp(blend, "multiply")) particle.blend = Multiply;
                            else if (!strcmp(blend, "screen")) particle.blend = BlendScreen;
                            else panic("%s: unrecognized blend mode %s", path.c_str(), blend);
                        } else if (!strcmp(keyName, "time")) {
                            particle.time = atof(yamlStringToCString(value->data.scalar.value));
                        } else if (!strcmp(keyName, "count")) {
                            particle.count = atoi(yamlStringToCString(value->data.scalar.value));
                        } else if (!strcmp(keyName, "aligned")) {
                            particle.aligned = !strcmp("true", yamlStringToCString(value->data.scalar.value));
                        } else if (!strcmp(keyName, "duration")) {
                            particle.duration = atof(yamlStringToCString(value->data.scalar.value));
                        } else if (!strcmp(keyName, "layer")) {
                            particle.layer = atoi(yamlStringToCString(value->data.scalar.value));
                        } else if (!strcmp(keyName, "props")) {
                            for (yaml_node_pair_t *it = value->data.mapping.pairs.start; it < value->data.mapping.pairs.top; ++it) {
                                yaml_node_t *keyNode = yaml_document_get_node(&doc, it->key);
                                char *keyName = yamlStringToCString(keyNode->data.scalar.value);
                                yaml_node_t *value = yaml_document_get_node(&doc, it->value);
                                #define GET_VAL(i) atof(yamlStringToCString(yaml_document_get_node(&doc, *(value->data.sequence.items.start + i))->data.scalar.value))
                                #define PARSE_STATIC(name) (particle.props.set##name(GET_VAL(0), GET_VAL(1)))
                                #define PARSE_DYNAMIC(name) (particle.props.set##name(GET_VAL(0), GET_VAL(1), GET_VAL(2), GET_VAL(3)))
                                if (!strcmp(keyName, "originX")) { PARSE_STATIC(OriginX); }
                                else if (!strcmp(keyName, "originY")) { PARSE_STATIC(OriginY); }
                                else if (!strcmp(keyName, "duration")) { PARSE_STATIC(Duration); }
                                else if (!strcmp(keyName, "direction")) { PARSE_STATIC(Direction); }
                                else if (!strcmp(keyName, "red")) { PARSE_DYNAMIC(Red); }
                                else if (!strcmp(keyName, "green")) { PARSE_DYNAMIC(Green); }
                                else if (!strcmp(keyName, "blue")) { PARSE_DYNAMIC(Blue); }
                                else if (!strcmp(keyName, "alpha")) { PARSE_DYNAMIC(Alpha); }
                                else if (!strcmp(keyName, "scale")) { PARSE_DYNAMIC(Scale); }
                                else if (!strcmp(keyName, "rotation")) { PARSE_DYNAMIC(Rotation); }
                                else if (!strcmp(keyName, "distance")) { PARSE_DYNAMIC(Distance); }
                                else if (!strcmp(keyName, "orthoDistance")) { PARSE_DYNAMIC(OrthoDistance); }
                                else {
                                    panic("%s: unexpected particle system YAML prop key: %s", path.c_str(), keyName);
                                }
                            }
                        } else {
                            panic("%s: unexpected particle system YAML particle key: %s", path.c_str(), keyName);
                        }
                    }
                }
                effects[effect->name] = effect;
            }
        } else {
            panic("%s: unexpected particle system YAML key: %s", path.c_str(), keyName);
        }
    }

    yaml_document_delete(&doc);
    yaml_parser_delete(&parser);
}

}
