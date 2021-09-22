#ifndef KRIT_TASKMANAGER
#define KRIT_TASKMANAGER

#include <SDL2/SDL_mutex.h>
#include <SDL2/SDL_thread.h>
#include <algorithm>
#include <functional>
#include <queue>
#include <stddef.h>
#include <string>

namespace krit {

struct RenderContext;
struct UpdateContext;

template <typename T> using AsyncTask = std::function<void(T &)>;
using UpdateTask = AsyncTask<UpdateContext>;
using RenderTask = AsyncTask<RenderContext>;

template <typename T> struct AsyncQueue {
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

    void push(T job) {
        SDL_LockMutex(lock);
        queue.push(job);
        SDL_CondBroadcast(available);
        SDL_UnlockMutex(lock);
    }

    bool pop(T *to);

private:
    std::queue<T> queue;
};

struct TaskManager {
    static TaskManager *instance;

    /**
     * Used only by the main/render threads who are the only owners of their
     * work queues. Not safe when multiple threads may perform work.
     */
    template <typename T>
    static void work(AsyncQueue<AsyncTask<T>> &queue, T &ctx) {
        size_t len = queue.size();
        AsyncTask<T> job;
        for (size_t i = 0; i < len; ++i) {
            if (queue.pop(&job)) {
                job(ctx);
            }
        }
    }

    size_t size;
    SDL_Thread **threads;
    UpdateContext &ctx;

    AsyncQueue<UpdateTask> mainQueue;
    AsyncQueue<RenderTask> renderQueue;
    AsyncQueue<UpdateTask> workQueue;

    bool killed = false;

    TaskManager(UpdateContext &ctx, size_t size) : size(size), ctx(ctx) {
        instance = this;
        threads = new SDL_Thread *[size];
        for (size_t i = 0; i < size; ++i) {
            std::string threadName = "worker" + std::to_string(i + 1);
            SDL_CreateThread(workerFunc, threadName.c_str(), this);
        }
    }

    void push(UpdateTask task) { workQueue.push(task); }
    void pushMain(UpdateTask task) { mainQueue.push(task); }
    void pushRender(RenderTask task) { renderQueue.push(task); }

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
