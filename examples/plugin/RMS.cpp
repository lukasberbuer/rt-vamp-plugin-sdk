#include "RMS.hpp"

#include <cmath>
#include <numeric>  // accumulate

OutputList RMS::getOutputDescriptors() const {
    OutputList       list;
    OutputDescriptor d;
    d.identifier  = "rms";
    d.name        = "RMS";
    d.description = "Root mean square of signal";
    d.unit        = "V";
    d.binCount    = 1;
    // use default values for extend and quantization
    list.push_back(d);
    return list;
}

bool RMS::initialise(unsigned int /* stepSize */, unsigned int /* blockSize */) {
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

const FeatureSet& RMS::process(InputBuffer inputBuffer, uint64_t /* nsec */) {
    auto  signal = std::get<TimeDomainBuffer>(inputBuffer);
    auto& result = getFeatureSet();

    const float sumSquares = std::accumulate(
        signal.begin(), signal.end(), 0.0f, square<float>()
    );
    const float rms = std::sqrt(sumSquares / signal.size());

    result[0][0] = rms;
    return result;
}
