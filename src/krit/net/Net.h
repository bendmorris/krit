#ifndef KRIT_NET
#define KRIT_NET

#include "krit/script/Promise.h"
#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>

namespace krit {

struct NetRequest {
    static NetRequest *create() { return new NetRequest(); }

    std::string url;
    std::vector<std::string> headers;
    std::string message;

    void setHeader(const std::string &s) {
        headers.push_back(s);
    }

    Promise get();
    Promise post();
};

struct NetResponse {
    static NetResponse *create() { return new NetResponse(); }
    int responseCode;
    std::string data;
};

struct Net {
    virtual ~Net() = default;
    virtual Promise get(const NetRequest &) = 0;
    virtual Promise post(const NetRequest &) = 0;
};

std::unique_ptr<Net> net();

}

#endif
