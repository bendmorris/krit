#include "krit/asset/Assets.h"
#include "krit/App.h"
#include "krit/io/Io.h"
#include "krit/utils/Panic.h"
#include <yaml.h>

namespace krit {

static std::unordered_map<std::string, AssetType> assetTypes{
    {"Image", ImageAsset},
    {"Atlas", AtlasAsset},
    {"Font", FontAsset},
    {"Text", TextAsset},
    {"SpineSkeleton", SpineSkeletonAsset},
    {"Sound", SoundAsset},
    {"Music", MusicAsset},
};

std::vector<AssetInfo> Assets::_assets;
std::unordered_map<std::string, std::vector<AssetId>> Assets::_byPath;

static char *yamlStr(yaml_node_t *node) {
    return static_cast<char *>(static_cast<void *>(node->data.scalar.value));
}

const AssetInfo &Assets::byPath(const std::string &path) {
    auto found = _byPath.find(path);
    if (found == _byPath.end()) {
        panic("unrecognized asset path: %s\n", path.c_str());
    }
    auto &options = found->second;
    if (options.size() == 1) {
        return byId(options[0]);
    } else {
        AssetId best = -1;
        for (auto id : options) {
            auto &a = byId(id);
            if (a.type == ImageAsset) {
                // TODO: create an ImageData that can dynamically choose the
                // right resolution
                return a;
            } else {
                panic("ambiguous asset path: %s\n", path.c_str());
            }
        }
        return byId(best);
    }
}

const AssetInfo &Assets::byId(AssetId id) { return _assets[id]; }

void Assets::init() {
    if (!IoRead::exists("assets/assets.yaml")) {
        panic("couldn't find asset manifest");
    }

    int len;
    char *manifest = IoRead::read("assets/assets.yaml", &len);
    yaml_parser_t parser;
    yaml_parser_initialize(&parser);
    yaml_parser_set_input_string(&parser, (unsigned char *)manifest, len);

    yaml_document_t doc;
    if (!yaml_parser_load(&parser, &doc)) {
        panic("failed to parse asset manifest");
    }

    yaml_node_t *root = yaml_document_get_root_node(&doc);
    if (root->type != YAML_SEQUENCE_NODE) {
        panic("asset manifest didn't contain a top-level list");
    }
    _assets.reserve(root->data.sequence.items.top -
                    root->data.sequence.items.start);
    for (yaml_node_item_t *it = root->data.sequence.items.start;
         it < root->data.sequence.items.top; ++it) {
        yaml_node_t *node = yaml_document_get_node(&doc, *it);
        if (node->type != YAML_MAPPING_NODE) {
            panic("asset manifest expected list of maps");
        }
        _assets.emplace_back();
        auto &a = _assets.back();
        a.id = _assets.size() - 1;
        for (yaml_node_pair_t *it = node->data.mapping.pairs.start;
             it < node->data.mapping.pairs.top; ++it) {
            yaml_node_t *keyNode = yaml_document_get_node(&doc, it->key);
            char *keyName = yamlStr(keyNode);
            yaml_node_t *value = yaml_document_get_node(&doc, it->value);
            if (!strcmp("type", keyName)) {
                a.type = assetTypes[yamlStr(value)];
            } else if (!strcmp("paths", keyName)) {
                for (yaml_node_item_t *it2 = value->data.sequence.items.start;
                     it2 < value->data.sequence.items.top; ++it2) {
                    yaml_node_t *pathNode = yaml_document_get_node(&doc, *it2);
                    _byPath[yamlStr(pathNode)].push_back(a.id);
                    if (it2 == value->data.sequence.items.start) {
                        a.path = yamlStr(pathNode);
                    }
                }
            } else if (!strcmp("width", keyName)) {
                a.properties.img.dimensions.x = atoi(yamlStr(value));
            } else if (!strcmp("height", keyName)) {
                a.properties.img.dimensions.y = atoi(yamlStr(value));
            } else if (!strcmp("realWidth", keyName)) {
                a.properties.img.realDimensions.x = atoi(yamlStr(value));
            } else if (!strcmp("realHeight", keyName)) {
                a.properties.img.realDimensions.y = atoi(yamlStr(value));
            } else if (!strcmp("scale", keyName)) {
                a.properties.img.scale = atof(yamlStr(value));
            }
        }
    }

    yaml_document_delete(&doc);
    yaml_parser_delete(&parser);

    IoRead::free(manifest);
}

}
