#include "SpectralRolloff.hpp"

#include <algorithm>  // transform
#include <numeric>  // accumulate

bool SpectralRolloff::initialise(uint32_t stepSize, uint32_t blockSize) {
    magnitude_.resize(blockSize / 2 + 1);
    initialiseFeatureSet();
    return true;
}

void SpectralRolloff::reset() {}

static void computeMagnitude(const auto& fft, auto& magnitude) {
    if (magnitude.size() != fft.size()) {
        magnitude.resize(fft.size());
    }
    std::transform(fft.begin(), fft.end(), magnitude.begin(), std::abs<float>);
}

static size_t getRolloffIndex(const auto& magnitude, float sumLimit) {
    float sum = 0.0f;
    for (size_t i = 0; i < magnitude.size(); ++i) {
        sum += magnitude[i];
        if (sum > sumLimit)
            return i;
    }
    return magnitude.size() - 1;
}

inline static float binToFrequency(float sampleRate, size_t nfft, size_t index) {
    return 0.5f * sampleRate * index / (nfft - 1);
}

const SpectralRolloff::FeatureSet& SpectralRolloff::process(
    InputBuffer inputBuffer, uint64_t nsec
) {
    auto fft = std::get<FrequencyDomainBuffer>(inputBuffer);
    computeMagnitude(fft, magnitude_);

    const float  sum          = std::accumulate(magnitude_.begin(), magnitude_.end(), 0.0f);
    const float  sumLimit     = getParameter("rolloff").value() * sum;
    const size_t indexRolloff = getRolloffIndex(magnitude_, sumLimit);
    const float  frequency    = binToFrequency(getInputSampleRate(), magnitude_.size(), indexRolloff);

    auto& result = getFeatureSet();
    result[0][0] = frequency;
    return result;
}
