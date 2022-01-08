#include "RMS.hpp"

#include <cmath>
#include <numeric>  // accumulate

bool RMS::initialise(uint32_t stepSize, uint32_t blockSize) {
    initialiseFeatureSet();
    return true;
}

void RMS::reset() {}

template <typename T>
struct square {
    T operator()(const T& left, const T& right) const {   
        return left + right * right;
    }
};

const RMS::FeatureSet& RMS::process(InputBuffer inputBuffer, uint64_t nsec) {
    auto signal = std::get<TimeDomainBuffer>(inputBuffer);

    const float sumSquares = std::accumulate(
        signal.begin(), signal.end(), 0.0f, square<float>()
    );
    const float rms = std::sqrt(sumSquares / signal.size());

    auto& result = getFeatureSet();
    result[0][0] = rms;
    return result;
}
