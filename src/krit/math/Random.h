#ifndef KRIT_MATH_RANDOM
#define KRIT_MATH_RANDOM

#include <cassert>
#include <iterator>
#include <random>
#include <stddef.h>
#include <vector>

namespace krit {

extern std::random_device rd;
extern std::mt19937 rng;

template <typename T> using WeightedCollection = std::vector<std::pair<T, int>>;

template <typename T> T weightedChoice(WeightedCollection<T> weights) {
    std::uniform_real_distribution<float> r(0, 1);
    T current = weights[0].first;
    int currentIndex = weights[0].second;
    for (size_t i = 1; i < weights.size(); ++i) {
        auto &next = weights[i];
        float cumulativeProbability = 0;
        int weight = next.second;
        if (weight > 0) {
            for (int j = 0; j < next.second; ++j) {
                cumulativeProbability +=
                    (1 - cumulativeProbability) * (1.0 / (++currentIndex));
            }
            if (r(rng) <= cumulativeProbability) {
                current = next.first;
            }
        }
    }
    return current;
}

template <typename T, typename... Args>
T weightedChoice(WeightedCollection<T> weights, int fn(T, int, Args...),
                 Args... args) {
    std::uniform_real_distribution<float> r(0, 1);
    T current = weights[0].first;
    int currentIndex = weights[0].second;
    for (size_t i = 1; i < weights.size(); ++i) {
        auto &next = weights[i];
        float cumulativeProbability = 0;
        int weight = next.second;
        weight = fn(next.first, weight, args...);
        if (weight > 0) {
            for (int j = 0; j < next.second; ++j) {
                cumulativeProbability +=
                    (1 - cumulativeProbability) * (1.0 / (++currentIndex));
            }
            if (r(rng) <= cumulativeProbability) {
                current = next.first;
            }
        }
    }
    return current;
}

}

#endif
