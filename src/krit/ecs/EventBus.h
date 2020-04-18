#ifndef KRIT_ECS_EVENTBUS
#define KRIT_ECS_EVENTBUS

#include <list>
#include <tuple>

using namespace std;
using namespace krit;

namespace krit {

template <typename EventType, typename... EventDataTypes> struct EventBus {
    tuple<list<EventDataTypes>...> events;
    tuple<list<EventDataTypes>...> nextEvents;

    template <size_t e> auto get() -> decltype(std::get<e>(events)) {
        return std::get<e>(this->events);
    }

    template <size_t e, typename... Args> void add(Args&&... args) {
        return std::get<e>(this->nextEvents).emplace_back(args...);
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
            events.clear();
            events.splice(events.begin(), nextEvents);
        }
};

}

#endif
