#include "krit/asset/TextureAtlas.h"
#include "krit/asset/AssetCache.h"
#include "krit/Assets.h"
#include <fstream>
#include <sstream>
#include <string>

std::pair<int, int> parseTuple(std::string &x) {
    int a = 0, b = 0;
    size_t index = 0;
    while (x[index] != ',') {
        a *= 10;
        a += x[index++] - '0';
    }
    index += 2;
    while (index < x.length()) {
        b *= 10;
        b += x[index++] - '0';
    }
    return std::make_pair(a, b);
}

TextureAtlas::TextureAtlas(AssetCache *assetCache, const std::string &path) {
    std::ifstream input(path);
    std::string line;
    while (std::getline(input, line)) {
        if (line.empty()) {
            continue;
        }
        // an atlas page
        std::string pageName = line;
        size_t lastSlash = path.rfind("/");
        std::string s = (lastSlash == std::string::npos) ? pageName : (path.substr(0, lastSlash) + "/" + pageName);
        std::shared_ptr<ImageData> image = std::static_pointer_cast<ImageData>(assetCache->get(Assets::byPath(s)));

        while (std::getline(input, line) && line.find(':') != std::string::npos) {}
        while (!line.empty()) {
            // an atlas region
            std::string regionName = line;
            IntRectangle rect;
            while (std::getline(input, line)) {
                size_t split = line.find(':');
                if (split == std::string::npos) {
                    break;
                }
                std::string key = line.substr(0, split);
                if (key == "  size") {
                    std::string size = line.substr(split + 2);
                    std::pair<int, int> wh = parseTuple(size);
                    rect.width = wh.first;
                    rect.height = wh.second;
                } else if (key == "  xy") {
                    std::string pos = line.substr(split + 2);
                    std::pair<int, int> xy = parseTuple(pos);
                    rect.x = xy.first;
                    rect.y = xy.second;
                }
            }
            ImageRegion region(image, rect);
            this->regions.insert(std::make_pair(regionName, region));
        }
    }
}
