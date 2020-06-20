#include "krit/asset/TextLoader.h"
#include "krit/io/Io.h"
#include <fstream>
#include <sstream>
#include <memory>

namespace krit {

std::shared_ptr<void> TextLoader::loadAsset(const AssetInfo &info) {
    char *content = IoRead::read(info.path);
    std::shared_ptr<char> buffer(content);
    return std::static_pointer_cast<void>(buffer);
}

}
