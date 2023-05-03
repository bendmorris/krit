#include "ScriptFinalizer.h"

namespace krit {

static void finalize(JSRuntime *rt, JSValue val) {
    FinalizerData *p = static_cast<FinalizerData*>(JS_GetOpaque(val, 0));
    if (p) {
        p->~FinalizerData();
    }
}

JSClassDef FinalizerData::classDefFinalizer = {
    .class_name = "Finalizer",
    .finalizer = finalize,
};

FinalizerData::~FinalizerData() {
    switch (static_cast<FinalizerMode>(data.index())) {
        case FinalizerMode::Owned: {
            auto &owned = std::get<OwnedData>(data);
            owned.dtor(owned.p);
            break;
        }
        default: {}
    }
    data = std::monostate();
}

}
