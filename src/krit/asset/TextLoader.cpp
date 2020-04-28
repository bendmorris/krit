#include "krit/asset/TextLoader.h"
#include <fstream>
#include <sstream>
#include <memory>

namespace krit {

std::shared_ptr<void> TextLoader::loadAsset(const AssetInfo &info) {
    std::ifstream tinput(info.path);
    std::shared_ptr<std::stringstream> buffer = std::make_shared<std::stringstream>();
    *buffer << tinput.rdbuf();
    return buffer;
}

}
