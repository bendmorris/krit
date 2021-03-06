#ifndef KRIT_UTILS
#define KRIT_UTILS

#include <cassert>
#include <iterator>
#include <random>
#include <stddef.h>
#include <vector>

namespace krit {

extern std::random_device rd;
extern std::mt19937 rng;

template <typename T> T clamp(T v, T min, T max) {
    assert(min <= max);
    if (v < min)
        return min;
    if (v > max)
        return max;
    return v;
}

template <typename T> T lerp(T v1, T v2, T mix) {
    if (mix <= 0)
        return v1;
    else if (mix >= 1)
        return v2;
    else
        return (1 - mix) * v1 + mix * v2;
}

float maybeLerp(float t, float f(float));

float noChange(float t);
float easeInOut(float t);
float easeOutBounce(float t);

/**
 * Smooth, S-shaped interpolation between 0 and 1.
 *
 * Providing a template argument will apply the function to its result,
 * N times, creating an increasingly steep slope.
 */
template <int N = 0> float smoothStep(float n) {
    return smoothStep<0>(smoothStep<N - 1>(n));
}

template <> float smoothStep<0>(float n);

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

size_t getPeakRss();
size_t getCurrentRss();

}

#endif
