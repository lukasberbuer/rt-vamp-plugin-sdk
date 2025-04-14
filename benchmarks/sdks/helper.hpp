#pragma once

#include <algorithm>  // generate
#include <random>
#include <span>

inline void randomize(std::span<float> vec) {
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
