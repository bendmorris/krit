#ifndef KRIT_SCRIPT_SCRIPTCLASS
#define KRIT_SCRIPT_SCRIPTCLASS

struct JSClassDef;

namespace krit {

struct ScriptEngine;

struct ScriptClassDef {
    void (*registerClass)(ScriptEngine *engine);
    JSClassDef *classDef;
};

typedef ScriptClassDef *ScriptClass;

}

#endif
