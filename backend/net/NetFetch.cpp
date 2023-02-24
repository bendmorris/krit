#include "krit/Engine.h"
#include "krit/TaskManager.h"
#include "krit/net/Net.h"
#include <emscripten/fetch.h>

namespace krit {

struct NetFetch : public Net {
    NetFetch() {}

    ~NetFetch() {}

    Promise get(const NetRequest &request) override {
        Promise promise{engine->script.ctx};
        // TODO
        // CURL *curl = curl_easy_init();
        // struct curl_slist *list = nullptr;
        // curl_easy_setopt(curl, CURLOPT_URL, request.url.c_str());
        // curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        // curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
        // for (auto &head : request.headers) {
        //     list = curl_slist_append(list, head.c_str());
        // }
        // curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
        // TaskManager::instance->push([promise, curl, list](UpdateContext &) {
        //     CURLcode res = curl_easy_perform(curl);
        //     curl_slist_free_all(list);
        //     if (res != CURLE_OK) {
        //         TaskManager::instance->pushMain(
        //             [curl, promise, res](UpdateContext &) {
        //                 promise.reject(curl_easy_strerror(res));
        //                 curl_easy_cleanup(curl);
        //             });
        //     } else {
        //         TaskManager::instance->pushMain(
        //             [curl, promise](UpdateContext &) {
        //                 promise.resolve(true);
        //                 curl_easy_cleanup(curl);
        //             });
        //     }
        // });
        return promise;
    }

    Promise post(const NetRequest &request) override {
        Promise promise{engine->script.ctx};
        // TODO
        // CURL *curl = curl_easy_init();
        // struct curl_slist *list = nullptr;
        // curl_easy_setopt(curl, CURLOPT_VERBOSE, true);
        // curl_easy_setopt(curl, CURLOPT_URL, request.url.c_str());
        // curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        // for (auto &head : request.headers) {
        //     list = curl_slist_append(list, head.c_str());
        //     printf("%s\n", head.c_str());
        // }
        // curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
        // curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, request.message.c_str());
        // printf("Body = %s\n", request.message.c_str());
        // curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
        // TaskManager::instance->push([promise, curl, list](UpdateContext &) {
        //     CURLcode res = curl_easy_perform(curl);
        //     curl_slist_free_all(list);
        //     if (res != CURLE_OK) {
        //         TaskManager::instance->pushMain(
        //             [curl, promise, res](UpdateContext &) {
        //                 promise.reject(curl_easy_strerror(res));
        //                 curl_easy_cleanup(curl);
        //             });
        //     } else {
        //         TaskManager::instance->pushMain(
        //             [curl, promise](UpdateContext &) {
        //                 promise.resolve(true);
        //                 curl_easy_cleanup(curl);
        //             });
        //     }
        // });
        return promise;
    }
};

std::unique_ptr<Net> net() { return std::unique_ptr<Net>(new NetFetch()); }

}
