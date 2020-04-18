#ifndef KRIT_BEHAVIOR_BEHAVIORTREEBUILDER
#define KRIT_BEHAVIOR_BEHAVIORTREEBUILDER

#include "krit/behavior/BehaviorTree.h"
#include "krit/utils/Panic.h"
#include <cassert>
#include <vector>

namespace krit {

enum CompositeType {
    Sequence,
    Selector,
    Inverter,
    Succeeder,
};

enum TreeBuilderOpType {
    Node,
    OpenComposite,
    CloseComposite,
};

template <typename Context> struct TreeBuilderOp {
    TreeBuilderOpType type;
    CompositeType compositeType;
    EvalFunction<Context> function;

    TreeBuilderOp(EvalFunction<Context> f): type(Node), function(f) {}
    TreeBuilderOp(CompositeType t, bool open = true): type(open ? OpenComposite : CloseComposite), compositeType(t) {}
};

template <typename Context> struct CompositeState {
    CompositeType type;
    BehaviorTreeNode<Context> *onSuccess = nullptr;
    BehaviorTreeNode<Context> *onFailure = nullptr;

    CompositeState(CompositeType type): type(type) {}
};

template <typename Context> struct TreeBuilder {
    using Self = TreeBuilder<Context>;

    BehaviorTree<Context> &tree;
    std::vector<TreeBuilderOp<Context>> ops;

    TreeBuilder(BehaviorTree<Context> &tree): tree(tree) {}

    Self &node(EvalFunction<Context> f) {
        ops.emplace_back(f);
        return *this;
    }

    Self &sequence() {
        ops.emplace_back(Sequence);
        return *this;
    }

    Self &select() {
        ops.emplace_back(Selector);
        return *this;
    }

    Self &invert() {
        ops.emplace_back(Inverter);
        return *this;
    }

    Self &always() {
        ops.emplace_back(Succeeder);
        return *this;
    }

    Self &end() {
        int level = 1;
        for (int i = ops.size() - 1; i >= 0; --i) {
            if (ops[i].type == CloseComposite) {
                ++level;
            } else if (ops[i].type == OpenComposite && (--level == 0)) {
                ops.emplace_back(ops[i].compositeType, false);
                return *this;
            }
        }
        panic("end() called without a composite");
    }

    bool build() {
        std::vector<CompositeState<Context>> compositeStack;
        compositeStack.emplace_back(Selector);
        BehaviorTreeNode<Context> *last = nullptr;
        while (!ops.empty()) {
            TreeBuilderOp<Context> op = ops.back();
            ops.pop_back();
            auto &composite = compositeStack.back();
            switch (op.type) {
                case Node: {
                    last = new BehaviorTreeNode<Context>(op.function);
                    last->onSuccess = composite.onSuccess;
                    last->onFailure = composite.onFailure;
                    switch (composite.type) {
                        case Selector: {
                            // on failure, continue
                            composite.onFailure = last;
                            break;
                        }
                        case Sequence: {
                            // on success, continue
                            composite.onSuccess = last;
                            break;
                        }
                        case Inverter: {
                            last->onSuccess = composite.onFailure;
                            last->onFailure = composite.onSuccess;
                            break;
                        }
                        case Succeeder: {
                            last->onSuccess = last->onFailure = composite.onSuccess;
                            break;
                        }
                    }
                    break;
                }
                case CloseComposite: {
                    compositeStack.emplace_back(op.compositeType);
                    auto &newComposite = compositeStack.back();
                    newComposite.onSuccess = composite.onSuccess;
                    newComposite.onFailure = composite.onFailure;
                    break;
                }
                case OpenComposite: {
                    compositeStack.pop_back();
                    auto &newComposite = compositeStack.back();
                    switch (newComposite.type) {
                        case Selector: {
                            newComposite.onFailure = last;
                            break;
                        }
                        case Sequence: {
                            newComposite.onSuccess = last;
                            break;
                        }
                    }
                    break;
                }
            }
        }

        assert(compositeStack.size() == 1);
        tree.root = last;
        return true;
    }
};

}

#endif
