#include "ObjectHeader.h"

#include "krit/utils/Log.h"
#include <cassert>
#include <vector>

namespace krit {

namespace {
std::vector<ObjectHeader *> objHeaderPool;
}

ObjectHeader::~ObjectHeader() { reset(); }

ObjectHeader *ObjectHeader::create() {
    if (objHeaderPool.size()) {
        ObjectHeader *result = objHeaderPool.back();
        objHeaderPool.pop_back();
        return result;
    }
    return new ObjectHeader();
}

ObjectHeader::ObjectHeader(ObjectHeader &&other) {
    value = std::move(other.value);
    other.reset();
}

void ObjectHeader::recycle(ObjectHeader *p) {
    p->reset();
    if (objHeaderPool.size() < 1024) {
        objHeaderPool.push_back(p);
    }
}

void *ObjectHeader::get() {
    switch (type()) {
        case OwnershipType::Unique: {
            return std::get<UniquePtr>(value).get();
        }
        case OwnershipType::Shared: {
            return std::get<std::shared_ptr<void>>(value).get();
        }
        case OwnershipType::Raw: {
            return std::get<void *>(value);
        }
        case OwnershipType::Empty: {
            return nullptr;
        }
    }
    Log::error("script", "invalid object ownership type: %i (%p)\n", static_cast<int>(type()), this);
    assert(false);
    return nullptr;
}

void ObjectHeader::reset() {
    value = std::monostate{};
}

}