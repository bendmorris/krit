#include "krit/TaskManager.h"

namespace krit {

TaskManager *TaskManager::instance = nullptr;

#if KRIT_ENABLE_THREADS
bool AsyncQueue::pop(AsyncTask *to) {
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

#else

bool AsyncQueue::pop(AsyncTask *to) {
    if (queue.empty()) {
        return false;
    }
    *to = queue.front();
    queue.pop();
    return true;
}

#endif

#if KRIT_ENABLE_THREADS
void TaskManager::workerLoop() {
    while (true) {
        AsyncTask job;
        bool popped = workQueue.pop(&job);
        if (TaskManager::instance->killed) {
            break;
        }
        if (popped) {
            job();
        }
    }
}
#endif

}
