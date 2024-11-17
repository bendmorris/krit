#include "krit/Engine.h"
#include "krit/io/Io.h"
#include "krit/utils/Panic.h"
#include <memory>
// #define ZIP_STATIC 1
#include <zip.h>

namespace krit {

struct IoZip : public Io {
    struct ArchiveInfo {
        // TODO: maybe use an ID here rather than the path
        std::filesystem::path path;
        zip_t *zip;
        bool enabled;
    };

    std::vector<ArchiveInfo> archives;
    std::vector<bool> enabled;
    bool initialized = false;

    IoZip() {}

    ~IoZip() {
        for (auto archive : archives) {
            zip_close(archive.zip);
        }
    }

    bool addRoot(const std::filesystem::path &path) override {
        if (hasRoot(path)) {
            return false;
        }
        if (engine->platform->exists(path)) {
            zip_source_t *source =
                zip_source_file_create(path.string().c_str(), 0, 0, nullptr);
            zip_t *archive = zip_open_from_source(source, ZIP_RDONLY, nullptr);
            archives.push_back(
                (ArchiveInfo){.path = path, .zip = archive, .enabled = true});
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
        addRoot("assets.zip");
    }

    std::string readFile(const std::filesystem::path &path) override {
        LOG_DEBUG("read file: %s", path.string().c_str());
        init();
        if (!archives.empty()) {
            auto s = path.string();
            zip_stat_t stat;
            zip_stat_init(&stat);
            for (auto &archive : archives) {
                if (!archive.enabled) {
                    continue;
                }
                zip_stat(archive.zip, s.c_str(), 0, &stat);
                int index = zip_name_locate(archive.zip, s.c_str(), 0);
                if (index == -1) {
                    continue;
                }
                std::string buffer;
                buffer.resize(stat.size);
                zip_file_t *f = zip_fopen_index(archive.zip, index, 0);
                if (!f) {
                    panic("failed to open file in zip archive: %s\n", s.c_str());
                }
                int bytes = stat.size;
                char *cur = buffer.data();
                while (bytes) {
                    int read = zip_fread(f, cur, bytes);
                    if (read == -1) {
                        panic("error reading file in zip archive: %s (error=%i)\n",
                              s.c_str(), read);
                    }
                    bytes -= read;
                    cur += read;
                }
                zip_fclose(f);
                return buffer;
            }
        }
        // FIXME: return optional instead of panic
        panic("failed to find file in zip archives: %s", path.string().c_str());
    }

    std::optional<std::filesystem::path>
    find(const std::filesystem::path &path) override {
        init();
        if (!archives.empty()) {
            auto s = path.string();
            for (auto &archive : archives) {
                if (!archive.enabled) {
                    continue;
                }
                int index = zip_name_locate(archive.zip, s.c_str(), 0);
                if (index != -1) {
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
                int index = zip_name_locate(archive.zip, s.c_str(), 0);
                if (index != -1) {
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
                zip_int64_t num_entries =
                    zip_get_num_entries(archive.zip, /*flags=*/0);
                for (zip_int64_t i = 0; i < num_entries; ++i) {
                    const char *name =
                        zip_get_name(archive.zip, i, /*flags=*/0);
                    if (!strncmp(name, s.c_str(), s.size())) {
                        results.push_back(name);
                    }
                }
            }
        }
        return results;
    }
};

std::unique_ptr<Io> io() { return std::unique_ptr<Io>(new IoZip()); }

}
