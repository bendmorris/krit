#include "krit/asset/TextureAtlas.h"
#include "krit/asset/AssetCache.h"
#include "krit/Assets.h"
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

pair<int, int> parseTuple(string &x) {
    int a = 0, b = 0;
    int index = 0;
    while (x[index] != ',') {
        a *= 10;
        a += x[index++] - '0';
    }
    index += 2;
    while (index < x.length()) {
        b *= 10;
        b += x[index++] - '0';
    }
    return make_pair(a, b);
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
        std::string s = (lastSlash == string::npos) ? pageName : (path.substr(0, lastSlash) + "/" + pageName);
        std::shared_ptr<ImageData> image = std::static_pointer_cast<ImageData>(assetCache->get(Assets::byPath(s)));

        while (std::getline(input, line) && line.find(':') != string::npos) {}
        while (!line.empty()) {
            // an atlas region
            string regionName = line;
            IntRectangle rect;
            int a, b;
            while (std::getline(input, line)) {
                size_t split = line.find(':');
                if (split == string::npos) {
                    break;
                }
                string key = line.substr(0, split);
                if (key == "  size") {
                    string size = line.substr(split + 2);
                    pair<int, int> wh = parseTuple(size);
                    rect.width = wh.first;
                    rect.height = wh.second;
                } else if (key == "  xy") {
                    string pos = line.substr(split + 2);
                    pair<int, int> xy = parseTuple(pos);
                    rect.x = xy.first;
                    rect.y = xy.second;
                }
            }
            ImageRegion region(image, rect);
            this->regions.insert(make_pair(regionName, region));
        }
    }
}
