#pragma once

#include "quickjs.h"

namespace krit {

struct OwnedValue {
    JSContext *ctx;
    JSValue val;

    OwnedValue(JSContext *ctx, JSValue val) : ctx(ctx), val(val) {
        JS_DupValue(ctx, val);
    }
    OwnedValue(const OwnedValue &other) : OwnedValue(other.ctx, other.val) {}
    OwnedValue(OwnedValue &&other) {
        val = other.val;
        ctx = other.ctx;
        other.val = JS_UNDEFINED;
    }
    ~OwnedValue() { JS_FreeValue(ctx, val); }

    JSValue &operator*() { return val; }
    const JSValue &operator*() const { return val; }
    JSValue *operator->() { return &val; }
    JSValue const *operator->() const { return &val; }
};

}
