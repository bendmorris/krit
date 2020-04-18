#ifndef KRIT_UTILS_POOL
#define KRIT_UTILS_POOL

#include <memory>
#include <stack>
#include <vector>

using namespace std;

namespace krit {

template <class T>
class Pool {
    private:
        struct ReturnToPool {
            explicit ReturnToPool(weak_ptr<Pool<T>*> pool): pool(pool) {}

            void operator()(T* ptr) {
                if (shared_ptr<Pool*> pool_ptr = pool.lock()) {
                    try {
                        (*pool_ptr.get())->add(unique_ptr<T>{ptr});
                        return;
                    } catch(...) {}
                }
                default_delete<T>{}(ptr);
            }
            private:
                weak_ptr<Pool<T>*> pool;
        };

    public:
        using PtrT = unique_ptr<T, ReturnToPool>;

        Pool(): this_ptr(new Pool<T>*(this)) {}
        virtual ~Pool(){}

        void add(std::unique_ptr<T> t) {
            pool.push(std::move(t));
        }

        PtrT acquire() {
            assert(!pool.empty());
            PtrT tmp(
                pool.top().release(),
                ReturnToPool{weak_ptr<Pool<T>*>{this_ptr}}
            );
            pool.pop();
            return move(tmp);
        }

        bool empty() const {
            return pool.empty();
        }

        size_t size() const {
            return pool.size();
        }

    private:
        shared_ptr<Pool<T>*> this_ptr;
        stack<unique_ptr<T>> pool;
};

}

#endif
