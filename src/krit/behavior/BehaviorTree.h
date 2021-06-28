#ifndef KRIT_BEHAVIOR_BEHAVIORTREE
#define KRIT_BEHAVIOR_BEHAVIORTREE

#include "krit/utils/Panic.h"
#include <functional>
#include <vector>

namespace krit {

enum BehaviorResult {
    InProgress = -1,
    Failure = 0,
    Success = 1,
};

template <typename Context>
using EvalFunction = std::function<BehaviorResult(Context &)>;

template <typename Context> struct BehaviorTree;

// FIXME: pool these
template <typename Context> struct BehaviorTreeNode {
    EvalFunction<Context> f;
    BehaviorTreeNode *onSuccess = nullptr;
    BehaviorTreeNode *onFailure = nullptr;

    BehaviorTreeNode() {}
    BehaviorTreeNode(EvalFunction<Context> f) : f(f) {}

    BehaviorResult operator()(Context &ctx) { return this->f(ctx); }
};

template <typename Context> struct BehaviorTree {
    BehaviorTreeNode<Context> *root;
    BehaviorTreeNode<Context> *last = nullptr;

    template <typename... Args> BehaviorTree(Args &&... args) {
        this->root = new BehaviorTreeNode<Context>(args...);
    }

    void evaluate(Context &ctx) {
        BehaviorTreeNode<Context> *next = this->last ? this->last : this->root;
        this->last = nullptr;
        while (next) {
            BehaviorResult result = (*next)(ctx);
            switch (result) {
                case Success: {
                    next = next->onSuccess;
                    break;
                }
                case Failure: {
                    next = next->onFailure;
                    break;
                }
                case InProgress: {
                    this->last = next;
                    next = nullptr;
                    break;
                }
            }
        }
    }

    void reset() { this->last = nullptr; }
};

}

#endif
