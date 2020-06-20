#ifndef KRIT_ASSET_TEXTLOADER
#define KRIT_ASSET_TEXTLOADER

#include "krit/asset/AssetLoader.h"

namespace krit {

static const std::string TEXT_TYPE = "txt";

struct TextLoader: public AssetLoader {
    AssetType type() override { return TextAsset; }
    std::shared_ptr<void> loadAsset(const AssetInfo &info) override;
    TextLoader() {}
};

}

#endif
