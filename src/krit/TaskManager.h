#ifndef KRIT_TASKMANAGER
#define KRIT_TASKMANAGER

#if KRIT_ENABLE_THREADS
#include <SDL2/SDL_mutex.h>
#include <SDL2/SDL_thread.h>
#endif
#include <algorithm>
#include <functional>
#include <queue>
#include <stddef.h>
#include <string>

namespace krit {

struct RenderContext;

using AsyncTask = std::function<void(void)>;

#if KRIT_ENABLE_THREADS

struct AsyncQueue {
    SDL_mutex *lock;
    SDL_cond *available;

    AsyncQueue() {
        lock = SDL_CreateMutex();
        available = SDL_CreateCond();
    }

    ~AsyncQueue() {
        SDL_DestroyCond(available);
        SDL_DestroyMutex(lock);
    }

    size_t size() {
        SDL_LockMutex(lock);
        size_t result = queue.size();
        SDL_UnlockMutex(lock);
        return result;
    }

    void push(AsyncTask job) {
        SDL_LockMutex(lock);
        queue.push(job);
        SDL_CondBroadcast(available);
        SDL_UnlockMutex(lock);
    }

    bool pop(AsyncTask *to);

private:
    std::queue<AsyncTask> queue;
};

#else

struct AsyncQueue {
    size_t size() { return queue.size(); }
    void push(AsyncTask job) { queue.push(job); }

    bool pop(AsyncTask *to);

private:
    std::queue<AsyncTask> queue;
};

#endif

struct TaskManager {
    static TaskManager *instance;

    /**
     * Used only by the main/render threads who are the only owners of their
     * work queues. Not safe when multiple threads may perform work.
     */
    static void work(AsyncQueue &queue) {
        size_t len = queue.size();
        AsyncTask job;
        for (size_t i = 0; i < len; ++i) {
            if (queue.pop(&job)) {
                job();
            }
        }
    }

    size_t size;
#if KRIT_ENABLE_THREADS
    SDL_Thread **threads;
#endif

    AsyncQueue mainQueue;
    AsyncQueue renderQueue;
#if KRIT_ENABLE_THREADS
    AsyncQueue workQueue;
#endif

    bool killed = false;

    TaskManager(size_t size) : size(size) {
        instance = this;
#if KRIT_ENABLE_THREADS
        threads = new SDL_Thread *[size];
        for (size_t i = 0; i < size; ++i) {
            std::string threadName = "worker" + std::to_string(i + 1);
            threads[i] = SDL_CreateThread(workerFunc, threadName.c_str(), this);
        }
#endif
    }

    void cleanup() {
#if KRIT_ENABLE_THREADS
        killed = true;
        for (size_t i = 0; i < size; ++i) {
            SDL_WaitThread(threads[i], nullptr);
        }
        delete[] threads;
#endif
    }

#if KRIT_ENABLE_THREADS
    void push(AsyncTask task) { workQueue.push(task); }
#else
    void push(AsyncTask task) { mainQueue.push(task); }
#endif
    void pushMain(AsyncTask task) { mainQueue.push(task); }
    void pushRender(AsyncTask task) { renderQueue.push(task); }

private:
    static int workerFunc(void *raw) {
        TaskManager *taskManager = static_cast<TaskManager *>(raw);
        taskManager->workerLoop();
        return 0;
    }

    void workerLoop();
};

}

#endif
