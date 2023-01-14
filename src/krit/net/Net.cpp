#include "Net.h"
#include "krit/Engine.h"

namespace krit {

Promise NetRequest::get() { return engine->net->get(*this); }
Promise NetRequest::post() { return engine->net->post(*this); }

}
