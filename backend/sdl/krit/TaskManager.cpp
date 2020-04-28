#include "krit/TaskManager.h"

namespace krit {

void TaskManager::workerLoop() {
    AsyncTask job = pop();
    job(ctx);
}

}
