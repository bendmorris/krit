#ifndef KRIT_UTILS
#define KRIT_UTILS

#include <cmath>
#include <functional>
#include <random>
#include <utility>
#include <vector>

using namespace std;

namespace krit {

extern random_device rd;
extern mt19937 rng;

template <typename T> T lerp(T v1, T v2, T mix) {
    if (mix <= 0) return v1;
    else if (mix >= 1) return v2;
    else return (1 - mix) * v1 + mix * v2;
}

double maybeLerp(double t, double f(double));

double noChange(double t);
double easeInOut(double t);
double easeOutBounce(double t);

/**
 * Smooth, S-shaped interpolation between 0 and 1.
 *
 * Providing a template argument will apply the function to its result,
 * N times, creating an increasingly steep slope.
 */
template <int N = 0> double smoothStep(double n) {
    return smoothStep<0>(smoothStep<N - 1>(n));
}

template <> double smoothStep<0>(double n);

template <typename T> using WeightedCollection = vector<pair<T, int>>;

template <typename T> T weightedChoice(WeightedCollection<T> weights) {
    uniform_real_distribution<double> r(0, 1);
    T current = weights[0].first;
    int currentIndex = weights[0].second;
    for (int i = 1; i < weights.size(); ++i) {
        auto &next = weights[i];
        double cumulativeProbability = 0;
        int weight = next.second;
        if (weight > 0) {
            for (int j = 0; j < next.second; ++j) {
                cumulativeProbability += (1 - cumulativeProbability) * (1.0 / (++currentIndex));
            }
            if (r(rng) <= cumulativeProbability) {
                current = next.first;
            }
        }
    }
    return current;
}

template <typename T, typename... Args> T weightedChoice(WeightedCollection<T> weights, int fn(T, int, Args...), Args... args) {
    uniform_real_distribution<double> r(0, 1);
    T current = weights[0].first;
    int currentIndex = weights[0].second;
    for (int i = 1; i < weights.size(); ++i) {
        auto &next = weights[i];
        double cumulativeProbability = 0;
        int weight = next.second;
        weight = fn(next.first, weight, args...);
        if (weight > 0) {
            for (int j = 0; j < next.second; ++j) {
                cumulativeProbability += (1 - cumulativeProbability) * (1.0 / (++currentIndex));
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
