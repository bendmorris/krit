#ifndef KRIT_ASSET_ASSET_LOADER
#define KRIT_ASSET_ASSET_LOADER

#include <memory>
#include <string>

using namespace std;

namespace krit {

class AssetLoader {
    public:
        virtual string assetType() = 0;
        virtual int initialSize() { return 16; }
        virtual shared_ptr<void> loadAsset(string id) = 0;
};

}

#endif
