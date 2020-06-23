#include "krit/input/InputContext.h"
#include <vector>

namespace krit {

void InputContext::update(UpdateContext &ctx) {
    invoke(this->onUpdate, &ctx);

    this->active.clear();

    if (this->enabled) {
        std::vector<InputEvent> &events = this->bindings.getEvents();
        for (InputEvent &event : events) {
            if (event.eventType == InputFinish) {
                this->active.erase(event.action);
            } else {
                this->active.insert(std::make_pair(event.action, event.level));
            }
            auto found = this->actionMappings.find(event.action);
            if (found != this->actionMappings.end()) {
                ActionContext actionCtx(event.action, event.level);
                switch (event.eventType) {
                    case InputStart: {
                        invoke(found->second.onStart, &actionCtx, &ctx);
                        if (found->second.delay > 0) {
                            found->second.timer = 1;
                        }
                        break;
                    }
                    case InputActive: {
                        if (found->second.timer > 0) {
                            found->second.timer -= ctx.elapsed / found->second.delay;
                        }
                        if (found->second.timer <= 0) {
                            invoke(found->second.onActive, &actionCtx, &ctx);
                            found->second.timer = 1;
                        }
                        break;
                    }
                    case InputFinish: {
                        invoke(found->second.onFinish, &actionCtx, &ctx);
                        break;
                    }
                }
            }
        }
    }
};

}
