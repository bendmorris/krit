#ifndef KRIT_TASKMANAGER
#define KRIT_TASKMANAGER

#include "krit/UpdateContext.h"
#include "krit/render/RenderContext.h"
#include "SDL2/SDL.h"
#include <functional>
#include <queue>
#include <string>

namespace krit {

template <typename T> using AsyncTask = std::function<void(T&)>;
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
    static TaskManager *instance;

    /**
     * Used only by the main/render threads who are the only owners of their
     * work queues. Not safe when multiple threads may perform work.
     */
    template <typename T> static void work(AsyncQueue<AsyncTask<T>> &queue, T &ctx) {
        size_t len;
        while (len = queue.size()) {
            for (int i = 0; i < len; ++i) {
                (queue.pop())(ctx);
            }
        }
    }

    size_t size;
    SDL_Thread **threads;
    UpdateContext &ctx;

    AsyncQueue<UpdateTask> mainQueue;
    AsyncQueue<RenderTask> renderQueue;
    AsyncQueue<UpdateTask> workQueue;

    TaskManager(UpdateContext &ctx, size_t size):
        ctx(ctx),
        size(size)
    {
        instance = this;
        threads = new SDL_Thread*[size];
        for (int i = 0; i < size; ++i) {
            std::string threadName = "worker" + std::to_string(i + 1);
            SDL_CreateThread(workerFunc, threadName.c_str(), this);
        }
    }

    void push(UpdateTask task) { workQueue.push(task); }
    void pushMain(UpdateTask task) { mainQueue.push(task); }
    void pushRender(RenderTask task) { renderQueue.push(task); }
    UpdateTask pop() { return workQueue.pop(); }
    UpdateTask popMain() { return mainQueue.pop(); }
    RenderTask popRender() { return renderQueue.pop(); }

    private:
        static int workerFunc(void *raw) {
            TaskManager *taskManager = static_cast<TaskManager*>(raw);
            taskManager->workerLoop();
            return 0;
        }

        void workerLoop();
};

}

#endif
