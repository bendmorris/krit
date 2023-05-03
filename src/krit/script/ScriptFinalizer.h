#ifndef KRIT_SCRIPT_SCRIPTFINALIZER
#define KRIT_SCRIPT_SCRIPTFINALIZER

#include "quickjs.h"
#include <memory>
#include <variant>

namespace krit {

typedef void (*DestructorFn)(void *p);

enum class FinalizerMode {
    Empty,
    Owned,
    Shared,
};

struct OwnedData {
    DestructorFn dtor;
    void *p;
};

struct SharedData {
    std::shared_ptr<void> p;
};

struct FinalizerData {
    static JSClassDef classDefFinalizer;

    std::variant<std::monostate, OwnedData, SharedData> data;

    virtual ~FinalizerData();

    void clear() { data = std::monostate(); }

    template <typename T> void own(void *d) {
        data = OwnedData{[](void *p) { delete static_cast<T *>(p); }, d};
    }

    template <typename T> void ownExplicitDestruct(void *d) {
        data = OwnedData{[](void *p) { static_cast<T *>(p)->~T(); }, d};
    }

    void share(std::shared_ptr<void> d) { data = SharedData{d}; }
};

}

#endif
