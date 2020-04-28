#ifndef KRIT_ASSET_ASSET_LOADER
#define KRIT_ASSET_ASSET_LOADER

#include <memory>
#include <string>

namespace krit {

struct AssetLoader {
    virtual const std::string &assetType() = 0;
    virtual int initialSize() { return 16; }
    virtual std::shared_ptr<void> loadAsset(const std::string &id) = 0;
};

}

#endif
