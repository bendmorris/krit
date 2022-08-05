#ifndef KRIT_DATA_MODEL
#define KRIT_DATA_MODEL

#include "krit/utils/Panic.h"
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

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

    ModelType &get(size_t index) { return this->data[index]; }

    bool has(const std::string &id) {
        return this->idMap.find(id) != this->idMap.end();
    }

    std::vector<ModelType> &all() { return this->data; }

    ModelType &operator[](int index) { return this->data[index]; }
    ModelType &operator[](const std::string &id) { return this->getById(id); }

    size_t count() { return this->data.size(); }

    void reserve(size_t n) { data.reserve(n); }
    ModelType &newItem() {
        this->data.emplace_back();
        ModelType &item = this->data.back();
        item.index = this->data.size() - 1;
        return item;
    }

    void finish(ModelType &item) { this->idMap[item.id] = item.index; }

private:
    std::vector<ModelType> data;
    std::unordered_map<std::string, int> idMap;
};

}

#endif
