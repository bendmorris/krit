#include "krit/TaskManager.h"

namespace krit {

TaskManager *TaskManager::instance = nullptr;

template <typename T> bool AsyncQueue<T>::pop(T *to) {
    bool result = false;
    SDL_LockMutex(lock);
    SDL_CondWaitTimeout(available, lock, 1);
    if (TaskManager::instance->killed) {
        SDL_UnlockMutex(lock);
        return false;
    }
    if (!queue.empty()) {
        *to = queue.front();
        queue.pop();
        result = true;
    }
    SDL_UnlockMutex(lock);
    return result;
}

// explicit instantiation required here
template struct AsyncQueue<UpdateTask>;
template struct AsyncQueue<RenderTask>;

void TaskManager::workerLoop() {
    while (true) {
        UpdateTask job;
        bool popped = workQueue.pop(&job);
        if (TaskManager::instance->killed) {
            break;
        }
        if (popped) {
            job(ctx);
        }
    }
}

}
