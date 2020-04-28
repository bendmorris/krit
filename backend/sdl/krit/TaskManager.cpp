#include "krit/TaskManager.h"

namespace krit {

TaskManager *TaskManager::instance = nullptr;

void TaskManager::workerLoop() {
    while (true) {
        UpdateTask job = pop();
        job(ctx);
    }
}

}
