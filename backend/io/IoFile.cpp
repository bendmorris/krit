#include "krit/Engine.h"
#include "krit/io/Io.h"
#include <memory>

namespace krit {

struct IoFile : public Io {
    struct ArchiveInfo {
        // TODO: maybe use an ID here rather than the path
        std::filesystem::path path;
        bool enabled;
    };

    std::vector<ArchiveInfo> archives;
    std::vector<bool> enabled;
    bool initialized = false;

    IoFile() {}

    bool addRoot(const std::filesystem::path &path) override {
        if (hasRoot(path)) {
            return false;
        }
        if (engine->platform->exists(path)) {
            LOG_INFO("adding asset root: %s\n", path.c_str());
            archives.push_back((ArchiveInfo){.path = path, .enabled = true});
            return true;
        }
        LOG_WARN("couldn't add missing asset root: %s\n", path.c_str());
        return false;
    }

    bool hasRoot(const std::filesystem::path &path) override {
        for (auto &a : archives) {
            if (a.path == path) {
                return true;
            }
        }
        return false;
    }

    bool enableRoot(const std::filesystem::path &path) override {
        for (auto &a : archives) {
            if (a.path == path) {
                if (!a.enabled) {
                    a.enabled = true;
                    return true;
                } else {
                    return false;
                }
            }
        }
        return false;
    }

    bool disableRoot(const std::filesystem::path &path) override {
        for (auto &a : archives) {
            if (a.path == path) {
                if (a.enabled) {
                    a.enabled = false;
                    return true;
                } else {
                    return false;
                }
            }
        }
        return false;
    }

    void init() {
        if (initialized) {
            return;
        }
        initialized = true;
        addRoot("assets");
    }

    std::string readFile(const std::filesystem::path &path) override {
        init();
        auto s = path.string();
        if (!archives.empty()) {
            for (auto &archive : archives) {
                if (!archive.enabled) {
                    continue;
                }
                auto combinedPath = archive.path / path;
                if (engine->platform->exists(combinedPath)) {
                    return engine->platform->readFile(combinedPath);
                }
            }
        }
        // FIXME: return optional instead of panic
        panic("no such file: %s", s.c_str());
    }

    std::optional<std::filesystem::path> find(const std::filesystem::path &path) override {
        init();
        if (!archives.empty()) {
            auto s = path.string();
            for (auto &archive : archives) {
                if (!archive.enabled) {
                    continue;
                }
                auto combinedPath = archive.path / path;
                if (engine->platform->exists(combinedPath)) {
                    return archive.path;
                }
            }
        }
        return {};
    }

    bool exists(const std::filesystem::path &path) override {
        init();
        if (!archives.empty()) {
            auto s = path.string();
            for (auto &archive : archives) {
                if (!archive.enabled) {
                    continue;
                }
                auto combinedPath = archive.path / path;
                if (engine->platform->exists(combinedPath)) {
                    return true;
                }
            }
        }
        return false;
    }

    std::vector<std::string>
    readDir(const std::filesystem::path &path) override {
        init();
        std::vector<std::string> results;
        if (!archives.empty()) {
            auto s = path.string();
            for (auto &archive : archives) {
                if (!archive.enabled) {
                    continue;
                }
                auto combinedPath = archive.path / path;
                for (auto &child : engine->platform->readDir(path)) {
                    results.push_back(child);
                }
            }
        }
        return results;
    }
};

std::unique_ptr<Io> io() { return std::unique_ptr<Io>(new IoFile()); }

}
