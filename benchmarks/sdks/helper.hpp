#pragma once

#include <algorithm>  // generate
#include <random>

#include <benchmark/benchmark.h>

inline void addRateCounter(benchmark::State& state, size_t samplesPerIteration = 1) {
    state.counters["rate"] = benchmark::Counter(
        static_cast<double>(state.iterations()) * static_cast<double>(samplesPerIteration),
        benchmark::Counter::kIsRate
    );
}

inline void randomize(std::vector<float>& vec) {
    std::default_random_engine       engine;
    std::uniform_real_distribution<> dist(-1.0f, 1.0f);
    std::generate(vec.begin(), vec.end(), [&] { return dist(engine); });
}

template <typename T>
struct square {
    T operator()(const T& left, const T& right) const {   
        return left + right * right;
    }
};
