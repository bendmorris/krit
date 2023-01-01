#include "Io.h"
#include "IoFileHelper.h"
#include "krit/utils/Panic.h"
#include <memory>
#define ZIP_STATIC 1
#include <zip.h>

namespace krit {

struct IoZip : public Io {
    std::unique_ptr<IoFile> file;
    zip_t *archive = nullptr;

    IoZip() {
        file = std::unique_ptr<IoFile>(new IoFile());
        zip_source_t *source =
            zip_source_file_create("assets.zip", 0, 0, nullptr);
        archive = zip_open_from_source(source, ZIP_RDONLY, nullptr);
    }

    ~IoZip() {
        if (archive) {
            zip_close(archive);
        }
    }

    char *read(const char *path, int *length = nullptr) override {
        if (!archive) {
            return file->read(path, length);
        }
        zip_stat_t stat;
        zip_stat_init(&stat);
        zip_stat(archive, path, 0, &stat);
        int index = zip_name_locate(archive, path, 0);
        if (index == -1) {
            return file->read(path, length);
        }
        char *buffer = (char *)alloc(stat.size + 1);
        zip_file_t *f = zip_fopen_index(archive, index, 0);
        if (!f) {
            panic("error opening file from archive: %s\n", path);
        }
        int bytes = stat.size;
        char *cur = buffer;
        while (bytes) {
            int read = zip_fread(f, cur, bytes);
            if (read == -1) {
                panic("error reading file from archive: %s\n", path);
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

    void write(const char *path, const char *buf, size_t size) override {
        file->write(path, buf, size);
    }

    bool exists(const char *path) override {
        if (!archive) {
            return file->exists(path);
        }
        int index = zip_name_locate(archive, path, 0);
        return index != -1 || file->exists(path);
    }

    void *alloc(size_t size) override { return file->alloc(size); }
    void *calloc(size_t size) override { return file->calloc(size); }
    void *realloc(void *p, size_t size) override {
        return file->realloc(p, size);
    }
    void free(void *p) override { file->free(p); }
};

std::unique_ptr<Io> io() { return std::unique_ptr<Io>(new IoZip()); }

}
