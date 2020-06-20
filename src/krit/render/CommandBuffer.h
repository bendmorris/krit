#ifndef KRIT_RENDER_COMMAND_BUFFER
#define KRIT_RENDER_COMMAND_BUFFER

#include <tuple>
#include <vector>

namespace krit {

template <typename... CommandTypes> struct CommandBuffer {
    std::vector<size_t> commandTypes;
    std::tuple<std::vector<CommandTypes>...> commands;

    template <size_t e> auto get() -> decltype(std::get<e>(commands)) {
        return std::get<e>(this->commands);
    }

    template <size_t e, typename... Args> auto emplace_back(Args&&... args) -> decltype(std::get<e>(commands)[0]) {
        this->commandTypes.push_back(e);
        auto &c = std::get<e>(this->commands);
        c.emplace_back(args...);
        return c.back();
    }

    void clear() {
        this->commandTypes.clear();
        this->_clearAll<0, CommandTypes...>(CommandTypes()...);
    }

    private:
        template <int i, typename Head, typename... Tail> void _clearAll(Head head, Tail... tail) {
            this->_clearAll<i, Head>(head);
            this->_clearAll<i + 1, Tail...>(tail...);
        }
        template <int i, typename Head> void _clearAll(Head head) {
            auto &commands = std::get<i>(this->commands);
            commands.clear();
        }
};

}

#endif
