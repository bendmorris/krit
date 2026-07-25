#pragma once

#if KRIT_ENABLE_SCRIPT

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
        ctx = other.ctx;
        val = other.val;
        other.val = JS_UNDEFINED;
    }
    ~OwnedValue() { JS_FreeValue(ctx, val); }
    OwnedValue &operator=(const OwnedValue &other) {
        ctx = other.ctx;
        val = JS_DupValue(ctx, other.val);
        return *this;
    }
    OwnedValue &operator=(OwnedValue &&other) {
        ctx = other.ctx;
        val = other.val;
        other.val = JS_UNDEFINED;
        return *this;
    }

    JSValue &operator*() { return val; }
    const JSValue &operator*() const { return val; }
    JSValue *operator->() { return &val; }
    JSValue const *operator->() const { return &val; }
};

}

#endif
