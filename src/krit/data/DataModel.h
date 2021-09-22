#ifndef KRIT_DATA_MODEL
#define KRIT_DATA_MODEL

#include "krit/utils/Panic.h"
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include <yaml.h>

namespace krit {

struct DataItem {
    int index = -1;
    std::string id;
};

template <typename ModelType> struct DataModel {
    virtual ~DataModel() = default;

    ModelType &getById(const std::string &id) {
        auto it = this->idMap.find(id);
        if (it == this->idMap.end()) {
            panic("unknown ID: %s\n", id.c_str());
        }
        return this->data[it->second];
    }

    bool has(const std::string &id) {
        return this->idMap.find(id) != this->idMap.end();
    }

    std::vector<ModelType> &all() { return this->data; }

    ModelType &operator[](int index) { return this->data[index]; }

    ModelType &operator[](const std::string &id) { return this->getById(id); }

    size_t count() { return this->data.size(); }

    static char *yamlStr(yaml_node_t *node) {
        return static_cast<char *>(
            static_cast<void *>(node->data.scalar.value));
    }

    static char *yamlStringToCString(yaml_char_t *s) {
        return static_cast<char *>(static_cast<void *>(s));
    }

    static std::pair<std::string, std::string>
    parseDelimited(const std::string &value, char delimiter) {
        size_t delimiterPos = value.find(delimiter);
        if (delimiterPos == std::string::npos) {
            std::string val = value;
            return std::make_pair(val, val);
        } else {
            return std::make_pair(value.substr(0, delimiterPos),
                                  value.substr(delimiterPos + 1));
        }
    }

    static void
    applyToMap(yaml_document_t *doc, yaml_node_t *node,
               std::function<void(std::string &, yaml_node_t *)> fn) {
        for (yaml_node_pair_t *it = node->data.mapping.pairs.start;
             it < node->data.mapping.pairs.top; ++it) {
            yaml_node_t *keyNode = yaml_document_get_node(doc, it->key);
            char *keyName = yamlStringToCString(keyNode->data.scalar.value);
            yaml_node_t *value = yaml_document_get_node(doc, it->value);
            std::string key(keyName);
            fn(key, value);
        }
    }

    static void applyToSequence(yaml_document_t *doc, yaml_node_t *node,
                                std::function<void(yaml_node_t *)> fn) {
        for (yaml_node_item_t *childIndex = node->data.sequence.items.start;
             childIndex < node->data.sequence.items.top; ++childIndex) {
            yaml_node_t *child = yaml_document_get_node(doc, *childIndex);
            fn(child);
        }
    }

    /**
     * Parse data from file at `path` into vector `this->data`. Can be called
     * multiple times. Calls `parseItem` for each element in the list generated
     * by parsing `path`.
     */
    void parse(const char *path) {
        // FIXME: should use IoRead and yaml_parser_set_input_string
        FILE *input = fopen(path, "rb");
        yaml_parser_t parser;
        yaml_parser_initialize(&parser);
        yaml_parser_set_input_file(&parser, input);
        yaml_document_t doc;
        if (!yaml_parser_load(&parser, &doc)) {
            panic("failed to parse data YAML: %s", path);
        }

        yaml_node_t *root = yaml_document_get_root_node(&doc);
        if (root->type != YAML_SEQUENCE_NODE) {
            panic("%s: data YAML didn't contain a top-level list", path);
        }
        for (yaml_node_item_t *it = root->data.sequence.items.start;
             it < root->data.sequence.items.top; ++it) {
            yaml_node_t *node = yaml_document_get_node(&doc, *it);
            if (node->type != YAML_MAPPING_NODE) {
                panic("%s: expected list of maps", path);
            }
            this->parseItem(&doc, node);
        }

        yaml_document_delete(&doc);
        yaml_parser_delete(&parser);
    }

    template <typename... Args> ModelType &newItem(Args &&... args) {
        this->data.emplace_back(args...);
        ModelType &item = this->data.back();
        item.index = this->data.size() - 1;
        return item;
    }

    void finish(ModelType &item) { this->idMap[item.id] = item.index; }

    /**
     * Given a single YAML node, emplace a data element into `data`.
     */
    virtual void parseItem(yaml_document_t *doc, yaml_node_t *node){};

private:
    std::vector<ModelType> data;
    std::unordered_map<std::string, int> idMap;
};

}

#endif
