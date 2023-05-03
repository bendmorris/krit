#include "krit/asset/AssetLoader.h"
#include "krit/Engine.h"
#include "krit/asset/AssetType.h"
#include "krit/io/Io.h"
#include <cassert>
#include <string>

namespace krit {

template <>
std::shared_ptr<std::string>
AssetLoader<std::string>::loadAsset(const std::string &path) {
    // load asset as raw text
    std::string content = engine->io->readFile(path.c_str());
    return std::make_shared<std::string>(std::move(content));
}

template <>
bool AssetLoader<std::string>::assetIsReady(std::string *txt) {
    return true;
}

template <> AssetType AssetLoader<std::string>::type() {
    return TextAsset;
}

template <> size_t AssetLoader<std::string>::cost(std::string *txt) {
    return txt->size();
}

}
