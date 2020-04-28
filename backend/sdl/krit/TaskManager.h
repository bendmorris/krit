#ifndef KRIT_TASKMANAGER
#define KRIT_TASKMANAGER

#include "krit/UpdateContext.h"
#include "SDL2/SDL.h"
#include <functional>
#include <queue>
#include <string>

namespace krit {

using AsyncTask = std::function<void(UpdateContext&)>;

template <typename T> struct AsyncQueue {
    SDL_mutex *lock;
    SDL_cond *available;

    AsyncQueue() {
        lock = SDL_CreateMutex();
        available = SDL_CreateCond();
    }

    ~AsyncQueue() {
        SDL_DestroyMutex(lock);
        SDL_DestroyCond(available);
    }

    size_t size() {
        SDL_LockMutex(lock);
        size_t result = queue.size();
        SDL_UnlockMutex(lock);
        return result;
    }

    void push(T job) {
        SDL_LockMutex(lock);
        queue.push(job);
        SDL_CondBroadcast(available);
        SDL_UnlockMutex(lock);
    }

    T pop() {
        SDL_LockMutex(lock);
        while (queue.empty()) {
            SDL_CondWait(available, lock);
        }
        T result = queue.front();
        queue.pop();
        SDL_UnlockMutex(lock);
        return result;
    }

    private:
        std::queue<T> queue;
};

struct TaskManager {
    size_t size;
    SDL_Thread **threads;
    UpdateContext &ctx;

    TaskManager(UpdateContext &ctx, size_t size):
        ctx(ctx),
        size(size)
    {
        threads = new SDL_Thread*[size];
        for (int i = 0; i < size; ++i) {
            std::string threadName = "worker" + std::to_string(i + 1);
            SDL_CreateThread(workerFunc, threadName.c_str(), this);
        }
    }

    void push(AsyncTask task) { queue.push(task); }
    AsyncTask pop() { return queue.pop(); }

    private:
        static int workerFunc(void *raw) {
            TaskManager *taskManager = static_cast<TaskManager*>(raw);
            taskManager->workerLoop();
            return 0;
        }

        AsyncQueue<AsyncTask> queue;

        void workerLoop();
};

}

#endif
