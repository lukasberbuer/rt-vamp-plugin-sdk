#include "FFT.hpp"

#include <stdexcept>

struct FFT::Impl {};

FFT::FFT() {
    throw std::logic_error("No FFT library found");
}

FFT::~FFT() = default;

FFT::FFT(FFT&&) noexcept = default;
FFT& FFT::operator=(FFT&&) noexcept = default;

void FFT::initialise(size_t blockSizeTimeDomain) {}

std::span<const std::complex<float>> FFT::compute(std::span<const float> buffer) {
    return {};
}
