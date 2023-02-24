#include "IoFileHelper.h"
#include "krit/Engine.h"
#include "krit/io/Io.h"
#include "krit/utils/Panic.h"
#include <memory>
#define ZIP_STATIC 1
#include <zip.h>

namespace krit {

struct IoZip : public Io {
    std::unique_ptr<IoFile> file;
    std::vector<zip_t *> archives;
    bool initialized = false;

    IoZip() {
        // used for writing and as a read fallback
        file = std::unique_ptr<IoFile>(new IoFile());
    }

    ~IoZip() {
        for (auto archive : archives) {
            zip_close(archive);
        }
    }

    void init() {
        if (initialized) {
            return;
        }
        initialized = true;
        if (file->exists("assets.zip")) {
            zip_source_t *source =
                zip_source_file_create("assets.zip", 0, 0, nullptr);
            auto archive = zip_open_from_source(source, ZIP_RDONLY, nullptr);
            archives.push_back(archive);
        }
    }

    char *read(const std::filesystem::path &path,
               int *length = nullptr) override {
        if (!archive) {
            return file->read(path, length);
        }
        auto s = path.string();
        zip_stat_t stat;
        zip_stat_init(&stat);
        for (auto archive : archives) {
            zip_stat(archive, s.c_str(), 0, &stat);
            int index = zip_name_locate(archive, s.c_str(), 0);
            if (index == -1) {
                return file->read(path, length);
            }
            char *buffer = (char *)alloc(stat.size + 1);
            zip_file_t *f = zip_fopen_index(archive, index, 0);
            if (!f) {
                panic("error opening file from archive: %s\n", s.c_str());
            }
        }
        int bytes = stat.size;
        char *cur = buffer;
        while (bytes) {
            int read = zip_fread(f, cur, bytes);
            if (read == -1) {
                panic("error reading file from archive: %s\n", s.c_str());
            }
            bytes -= read;
            cur += read;
        }
        buffer[stat.size] = 0;
        if (length) {
            *length = stat.size;
        }
        zip_fclose(f);
        return buffer;
    }

    void write(const std::filesystem::path &path, const char *buf,
               size_t size) override {
        file->write(path.c_str(), buf, size);
    }

    bool exists(const std::filesystem::path &path) override {
        init();
        if (!archive) {
            return file->exists(path);
        }
        auto s = path.string();
        int index = zip_name_locate(archive, s.c_str(), 0);
        return index != -1 || file->exists(path);
    }
    bool isDirectory(const std::filesystem::path &path) override {
        return file->isDirectory(path);
    }

    bool rm(const std::filesystem::path &path) override {
        return file->rm(path);
    }

    void *alloc(size_t size) override { return file->alloc(size); }
    void *calloc(size_t size) override { return file->calloc(size); }
    void *realloc(void *p, size_t size) override {
        return file->realloc(p, size);
    }
    void free(void *p) override { file->free(p); }
    bool mkdir(const std::filesystem::path &path) override {
        return file->mkdir(path);
    }
};

std::unique_ptr<Io> io() { return std::unique_ptr<Io>(new IoZip()); }

}
