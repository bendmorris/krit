#ifndef KRIT_UTILS_LRU
#define KRIT_UTILS_LRU

#include <functional>

namespace krit {

template <typename K, typename V> struct LruNode {
    bool active = false;
    K key;
    V value;
    LruNode<K, V> *prev = nullptr;
    LruNode<K, V> *next = nullptr;

    LruNode<K, V>() {}
    LruNode<K, V>(K &key, V &value): key(key), value(value) {}
};

template <typename K, typename V, int N> struct Lru {
    LruNode<K, V> data[N];
    LruNode<K, V> *head = nullptr;
    std::function<void(K &key, V &value)> onUnload = nullptr;

    Lru<K, V, N>() {
        for (int i = 0; i < N; ++i) {
            data[i].active = false;
        }
    }

    bool exists(K &key) {
        LruNode<K, V> *cur = head;
        while (cur) {
            if (cur.active && cur.key == key) {
                return true;
            }
            cur = cur->next;
        }
        return false;
    }

    V *get(K &key) {
        LruNode<K, V> *cur = head;
        while (cur) {
            if (cur->active && cur->key == key) {
                // found it
                if (cur != head) {
                    // move this node to the front
                    cur->prev->next = cur->next;
                    if (cur->next) {
                        cur->next->prev = cur->prev;
                    }
                    head->prev = cur;
                    cur->next = head;
                    head = cur;
                }
                return &cur->value;
            }
            cur = cur->next;
        }
        return nullptr;
    }

    V &put(K &key) {
        LruNode<K, V> *newPosition = nullptr;
        for (int i = 0; i < N; ++i) {
            if (!data[i].active) {
                newPosition = &data[i];
                break;
            }
        }
        if (!newPosition) {
            // we need to bump off the current tail
            LruNode<K, V> *tail = &data[0];
            while (tail->next) {
                tail = tail->next;
            }
            tail->prev->next = nullptr;
            if (onUnload) {
                onUnload(tail->key, tail->value);
            }
            newPosition = tail;
        }
        // the new elemt is the new head
        newPosition->active = true;
        newPosition->key = key;
        if (this->head) {
            this->head->prev = newPosition;
        }
        newPosition->next = head;
        newPosition->prev = nullptr;
        head = newPosition;
        return newPosition->value;
    }
};

}

#endif
