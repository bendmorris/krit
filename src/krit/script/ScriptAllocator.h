#ifndef KRIT_SCRIPTALLOCATOR
#define KRIT_SCRIPTALLOCATOR

#include "quickjs.h"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>

namespace krit {

struct Cell {
    static Cell *alloc(size_t size) {
        Cell *c = (Cell *)malloc(offsetof(Cell, data[size]));
        return c;
    }

    template <size_t SIZE> static size_t cellSize() {
        return offsetof(Cell, data[SIZE]);
    }

    void *getData() { return (void *)data; }

    union {
        Cell *next;
        uint8_t block;
    };
    char data[1];
};

struct OversizedCell {
    static OversizedCell *alloc(size_t size) {
        OversizedCell *o =
            (OversizedCell *)malloc(offsetof(OversizedCell, cell.data[size]));
        o->size = size;
        o->cell.block = 7;
        return o;
    }

    size_t size;
    Cell cell;
};

template <size_t SIZE> struct BlockAllocator {
    static const size_t PAGE_SIZE = 0x40000;

    BlockAllocator(uint8_t id) {
        this->id = id;
        storage = Cell::alloc(PAGE_SIZE);
        storage->next = nullptr;
    }
    ~BlockAllocator() {
        while (this->storage) {
            Cell *next = this->storage->next;
            ::free(this->storage);
            this->storage = next;
        }
    }

    size_t cellSize() { return SIZE; }

    void *alloc() {
        if (!next) {
            static_assert(PAGE_SIZE > SIZE * 32,
                          "page size is too small for this block size");
            if (storageIndex + Cell::cellSize<SIZE>() > PAGE_SIZE) {
                // puts("need new storage");
                Cell *newStorage = Cell::alloc(PAGE_SIZE);
                newStorage->next = storage;
                storage = newStorage;
                storageIndex = 0;
            }
            for (size_t i = 0; i < 32; ++i) {
                Cell *newCell = (Cell *)(void *)(&storage->data[storageIndex]);
                newCell->next = next;
                next = newCell;
                storageIndex += Cell::cellSize<SIZE>();
                if (storageIndex + Cell::cellSize<SIZE>() > PAGE_SIZE) {
                    break;
                }
            }
        }
        Cell *allocatedCell = next;
        next = next->next;
        allocatedCell->block = this->id;
        // printf("returning: %p\n", allocatedCell->getData());
        return allocatedCell->getData();
    }

    void free(Cell *cell) {
        assert(cell->block == id);
        cell->next = next;
        next = cell;
    }

    uint8_t id;
    Cell *next = nullptr;
    Cell *storage = nullptr;
    size_t storageIndex = 0;
};

struct ScriptAllocator {
    static Cell *getCell(void *data) {
        return (Cell *)(void *)((char *)data - offsetof(Cell, data));
    }

    static OversizedCell *getOversized(Cell *cell) {
        return (OversizedCell *)(void *)((char *)cell -
                                         offsetof(OversizedCell, cell));
    }

    static void *alloc(JSMallocState *s, size_t size) {
        // printf("alloc %zu\n", size);
        printf("%zu\n", s->malloc_size);
        s->malloc_count++;
        if (size <= 8) {
            s->malloc_size += a8.cellSize();
            return a8.alloc();
        } else if (size <= 16) {
            s->malloc_size += a16.cellSize();
            return a16.alloc();
        } else if (size <= 32) {
            s->malloc_size += a32.cellSize();
            return a32.alloc();
        } else if (size <= 64) {
            s->malloc_size += a64.cellSize();
            return a64.alloc();
        } else if (size <= 128) {
            s->malloc_size += a128.cellSize();
            return a128.alloc();
        } else if (size <= 256) {
            s->malloc_size += a256.cellSize();
            return a256.alloc();
        } else if (size <= 512) {
            s->malloc_size += a512.cellSize();
            return a512.alloc();
        } else {
            OversizedCell *o = OversizedCell::alloc(size);
            // printf("returning oversized cell %p of size %zu\n", o, o->size);
            s->malloc_size += size;
            return o->cell.getData();
        }
    }

    static void free(JSMallocState *s, void *p) {
        if (!p) {
            return;
        }
        // printf("free %p\n", p);
        --s->malloc_count;
        Cell *cell = getCell(p);
        switch (cell->block) {
            case 0: {
                s->malloc_size -= a8.cellSize();
                a8.free(cell);
                break;
            }
            case 1: {
                s->malloc_size -= a16.cellSize();
                a16.free(cell);
                break;
            }
            case 2: {
                s->malloc_size -= a32.cellSize();
                a32.free(cell);
                break;
            }
            case 3: {
                s->malloc_size -= a64.cellSize();
                a64.free(cell);
                break;
            }
            case 4: {
                s->malloc_size -= a128.cellSize();
                a128.free(cell);
                break;
            }
            case 5: {
                s->malloc_size -= a256.cellSize();
                a256.free(cell);
                break;
            }
            case 6: {
                s->malloc_size -= a512.cellSize();
                a512.free(cell);
                break;
            }
            case 7: {
                OversizedCell *o = getOversized(cell);
                s->malloc_size -= o->size;
                ::free(o);
                break;
            }
            default: {
                assert(false);
            }
        }
    }

    static size_t cellSize(Cell *c) {
        switch (c->block) {
            case 0:
                return a8.cellSize();
            case 1:
                return a16.cellSize();
            case 2:
                return a32.cellSize();
            case 3:
                return a64.cellSize();
            case 4:
                return a128.cellSize();
            case 5:
                return a256.cellSize();
            case 6:
                return a512.cellSize();
            case 7: {
                OversizedCell *o = getOversized(c);
                return o->size;
            }
        }
        assert(false);
        return 0;
    }

    static void *realloc(JSMallocState *s, void *ptr, size_t size) {
        // printf("realloc %p %zu\n", ptr, size);
        if (!ptr) {
            return alloc(s, size);
        } else if (!size) {
            free(s, ptr);
            return nullptr;
        }
        Cell *cell = getCell(ptr);
        size_t oldSize = cellSize(cell);
        // printf("%p size %zu -> %zu\n", cell, oldSize, size);
        if (size > 512) {
            // we're reallocating to an OversizedCell
            if (oldSize > 512) {
                OversizedCell *o = getOversized(cell);
                o = (OversizedCell *)::realloc(
                    o, offsetof(OversizedCell, cell.data[size]));
                s->malloc_size += size - oldSize;
                return o->cell.getData();
            } else {
                OversizedCell *o = OversizedCell::alloc(size);
                std::memcpy(o->cell.getData(), cell->getData(),
                            std::min<size_t>(size, oldSize));
                free(s, cell->getData());
                return o->cell.getData();
            }
        }
        if (size != oldSize) {
            void *newData = alloc(s, size);
            // Cell *newCell = getCell(newData);
            std::memcpy(newData, cell->getData(),
                        std::min<size_t>(size, oldSize));
            free(s, cell->getData());
            return newData;
        } else {
            return ptr;
        }
    }

    static BlockAllocator<8> a8;
    static BlockAllocator<16> a16;
    static BlockAllocator<32> a32;
    static BlockAllocator<64> a64;
    static BlockAllocator<128> a128;
    static BlockAllocator<256> a256;
    static BlockAllocator<512> a512;
};

}

#endif
