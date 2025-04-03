#include "FFT.hpp"

#include <cmath>
#include <numbers>

// reference: https://en.wikipedia.org/wiki/Window_function
static std::vector<float> cosineSum(size_t length, float a0, float a1, float a2, float a3) {
    using namespace std::numbers;

    std::vector<float> window(length);
    for (size_t i = 0; i < length; ++i) {
        const auto factor = pi_v<float> * static_cast<float>(i) / static_cast<float>(length - 1);
        window[i] = (
            a0
            - a1 * std::cos(2.0F * factor)
            + a2 * std::cos(4.0F * factor)
            - a3 * std::cos(6.0F * factor)
        );
    }
    return window;
}

std::vector<float> hanning(size_t length) {
    return cosineSum(length, 0.5, 0.5, 0, 0);
}

std::vector<float> hamming(size_t length) {
    return cosineSum(length, 0.54F, 0.46F, 0.0F, 0.0F);
}
