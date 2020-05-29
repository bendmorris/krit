#ifndef KRIT_ECS_EVENTBUS
#define KRIT_ECS_EVENTBUS

#include <functional>
#include <list>
#include <tuple>
#include <type_traits>
#include <vector>

namespace krit {

template <typename EventType, typename... EventDataTypes> struct EventBus {
    std::tuple<std::list<EventDataTypes>...> events;
    std::tuple<std::list<EventDataTypes>...> nextEvents;
    std::tuple<std::vector<std::function<bool(EventDataTypes)>>...> callbacks;

    template <size_t e> auto get() -> decltype(std::get<e>(events)) {
        return std::get<e>(this->events);
    }

    template <size_t e, typename... Args> auto add(Args&&... args) -> decltype(std::get<e>(events).back()) {
        std::get<e>(this->nextEvents).emplace_back(args...);
        return std::get<e>(this->nextEvents).back();
    }

    template <size_t e> void listen(typename std::remove_reference<decltype(std::get<e>(callbacks).back())>::type callback) {
        std::get<e>(this->callbacks).push_back(callback);
    }

    void clear() {
        this->_clearAll<0, EventDataTypes...>(EventDataTypes()...);
    }

    void step() {
        this->_stepAll<0, EventDataTypes...>(EventDataTypes()...);
    }

    private:
        template <int i, typename Head, typename... Tail> void _clearAll(Head head, Tail... tail) {
            this->_clearAll<i, Head>(head);
            this->_clearAll<i + 1, Tail...>(tail...);
        }
        template <int i, typename Head> void _clearAll(Head head) {
            auto &events = std::get<i>(this->events);
            events.clear();
        }
        template <int i, typename Head, typename... Tail> void _stepAll(Head head, Tail... tail) {
            this->_stepAll<i, Head>(head);
            this->_stepAll<i + 1, Tail...>(tail...);
        }
        template <int i, typename Head> void _stepAll(Head head) {
            auto &events = std::get<i>(this->events);
            auto &nextEvents = std::get<i>(this->nextEvents);
            auto &callbacks = std::get<i>(this->callbacks);
            events.clear();
            events.splice(events.begin(), nextEvents);
            // call any callbacks with the new events
            for (size_t c = 0; c < callbacks.size(); ++c) {
                auto &callback = callbacks[c];
                bool done = false;
                for (auto &event : events) {
                    if (!callback(event)) {
                        done = true;
                        break;
                    }
                }
                if (done) {
                    for (size_t j = c + 1; j < callbacks.size(); ++j) {
                        std::swap(callbacks[c], callbacks[j]);
                    }
                    callbacks.pop_back();
                    --c;
                    break;
                }
            }
        }
};

}

#endif
