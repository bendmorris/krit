#ifndef KRIT_SCRIPTALLOCATOR
#define KRIT_SCRIPTALLOCATOR

#include "quickjs.h"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#if TRACY_ENABLE
#include "Tracy.hpp"
#endif

namespace krit {

struct Cell {
    static Cell *alloc(size_t size) {
        Cell *c = (Cell *)malloc(offsetof(Cell, data) + size);
        return c;
    }

    template <size_t SIZE> static size_t cellSize() {
        return (offsetof(Cell, data[SIZE]) + 7) / 8 * 8;
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
            (OversizedCell *)malloc(offsetof(OversizedCell, cell.data) + size);
        o->size = size;
        o->cell.block = 0xff;
        #if TRACY_ENABLE
        TracyAlloc(o->cell.getData(), size);
        #endif
        return o;
    }

    size_t size;
    Cell cell;
};

template <size_t SIZE> struct BlockAllocator {
    static const size_t pageSize = 0x100000;

    BlockAllocator(uint8_t id) {
        this->id = id;
        storage = Cell::alloc(pageSize);
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
            static_assert(pageSize > SIZE * 32,
                          "page size is too small for this block size");
            if (storageIndex + Cell::cellSize<SIZE>() > pageSize) {
                // puts("need new storage");
                Cell *newStorage = Cell::alloc(pageSize);
                newStorage->next = storage;
                storage = newStorage;
                storageIndex = 0;
            }
            for (size_t i = 0; i < 32; ++i) {
                Cell *newCell = (Cell *)(void *)(&storage->data[storageIndex]);
                newCell->next = next;
                next = newCell;
                storageIndex += Cell::cellSize<SIZE>();
                if (storageIndex + Cell::cellSize<SIZE>() > pageSize) {
                    break;
                }
            }
        }
        Cell *allocatedCell = next;
        next = next->next;
        allocatedCell->block = this->id;
        // printf("returning: %p\n", allocatedCell->getData());
        #if TRACY_ENABLE
        TracyAlloc(allocatedCell->getData(), SIZE);
        #endif
        return allocatedCell->getData();
    }

    void free(Cell *cell) {
        #if TRACY_ENABLE
        TracyFree(cell->getData());
        #endif
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
        // printf("%zu\n", s->malloc_size);
        s->malloc_count++;
        if (size <= a1.cellSize()) {
            s->malloc_size += a1.cellSize();
            return a1.alloc();
        } else if (size <= a2.cellSize()) {
            s->malloc_size += a2.cellSize();
            return a2.alloc();
        } else if (size <= a3.cellSize()) {
            s->malloc_size += a3.cellSize();
            return a3.alloc();
        } else if (size <= a4.cellSize()) {
            s->malloc_size += a4.cellSize();
            return a4.alloc();
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
                s->malloc_size -= a1.cellSize();
                a1.free(cell);
                break;
            }
            case 1: {
                s->malloc_size -= a2.cellSize();
                a2.free(cell);
                break;
            }
            case 2: {
                s->malloc_size -= a3.cellSize();
                a3.free(cell);
                break;
            }
            case 3: {
                s->malloc_size -= a4.cellSize();
                a4.free(cell);
                break;
            }
            case 0xff: {
                OversizedCell *o = getOversized(cell);
                s->malloc_size -= o->size;
                #if TRACY_ENABLE
                TracyFree(o->cell.getData());
                #endif
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
                return a1.cellSize();
            case 1:
                return a2.cellSize();
            case 2:
                return a3.cellSize();
            case 3:
                return a4.cellSize();
            case 0xff: {
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
        if (size > a4.cellSize()) {
            // we're reallocating to an OversizedCell
            if (oldSize > a4.cellSize()) {
                OversizedCell *o = getOversized(cell);
                #if TRACY_ENABLE
                TracyFree(o->cell.getData());
                #endif
                o = (OversizedCell *)::realloc(
                    o, offsetof(OversizedCell, cell.data) + size);
                #if TRACY_ENABLE
                TracyAlloc(o->cell.getData(), size);
                #endif
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

    static BlockAllocator<16> a1;
    static BlockAllocator<64> a2;
    static BlockAllocator<256> a3;
    static BlockAllocator<1024> a4;
};

}

#endif
