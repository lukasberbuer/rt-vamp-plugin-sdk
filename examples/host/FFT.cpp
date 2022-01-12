#include "FFT.hpp"

#include <cmath>
#include <numbers>

// reference: https://en.wikipedia.org/wiki/Window_function
static std::vector<float> cosineSum(size_t length, float a0, float a1, float a2, float a3) {
    using namespace std::numbers;

    std::vector<float> window(length);
    for (size_t i = 0; i < length; ++i) {
        window[i] = (
            a0
            - a1 * std::cos(2.0f * pi_v<float> * i / (length - 1))
            + a2 * std::cos(4.0f * pi_v<float> * i / (length - 1))
            - a3 * std::cos(6.0f * pi_v<float> * i / (length - 1))
        );
    }
    return window;
}

std::vector<float> hanning(size_t length) {
    return cosineSum(length, 0.5, 0.5, 0, 0);
}

std::vector<float> hamming(size_t length) {
    return cosineSum(length, 0.54f, 0.46f, 0.0f, 0.0f);
}
