#include "Io.h"
#include "IoFileHelper.h"
#include <memory>

namespace krit {

std::unique_ptr<Io> io() {
    return std::unique_ptr<Io>(new IoFile());
}

}
