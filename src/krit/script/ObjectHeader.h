#pragma once

#include "quickjs.h"
#include <cassert>
#include <memory>
#include <variant>

namespace krit {

struct ObjectHeader {
    using UniquePtr = std::unique_ptr<void, void (*)(void *)>;
    static ObjectHeader *create();
    static void recycle(ObjectHeader *);

    static ObjectHeader *header(JSValue obj) {
        JSClassID _id;
        void *p = JS_GetAnyOpaque(obj, &_id);
        return static_cast<ObjectHeader *>(p);
    }
    template <typename T> static T *data(JSValue obj) {
        if (JS_IsUndefined(obj) || JS_IsNull(obj) || !JS_IsObject(obj)) {
            return nullptr;
        }
        return static_cast<T *>(ObjectHeader::header(obj)->get());
    }

    enum class OwnershipType {
        Empty,
        Unique,
        Shared,
        Raw,
    };

    ObjectHeader() : value(std::monostate{}) {}
    ObjectHeader(const ObjectHeader &other) = delete;
    ObjectHeader(ObjectHeader &&other);
    ~ObjectHeader();

    OwnershipType type() { return static_cast<OwnershipType>(value.index()); }
    void *get();
    template <typename T> std::shared_ptr<T> getShared() {
        assert(type() == OwnershipType::Shared);
        return std::static_pointer_cast<T>(
            std::get<std::shared_ptr<void>>(value));
    }

    template <typename T> std::unique_ptr<T> takeUnique() {
        assert(type() == OwnershipType::Unique);
        std::unique_ptr<T> val(std::get<UniquePtr>(value).release());
        reset();
        return val;
    }

    template <typename T> void setUnique(std::unique_ptr<T> &&ptr) {
        value = UniquePtr(ptr.release(),
                          [](void *p) { delete static_cast<T *>(p); });
    };
    void setShared(const std::shared_ptr<void> &ptr) { value = ptr; }
    void setRaw(void *ptr) { value = ptr; }

    void reset();

private:
    std::variant<std::monostate, UniquePtr, std::shared_ptr<void>, void *>
        value;
};

}
