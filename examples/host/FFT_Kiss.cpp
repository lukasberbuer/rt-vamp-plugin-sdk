#include "FFT.hpp"

#include <vector>

#include <kissfft/kiss_fftr.h>

struct FFT::Impl {
    explicit Impl(size_t blockSizeTimeDomain)
        : config(kiss_fftr_alloc(blockSizeTimeDomain, 0, nullptr, nullptr)),
          outputComplex(blockSizeTimeDomain / 2 + 1) {}

    ~Impl() {
        kiss_fftr_free(config);
    }

    kiss_fftr_state*          config{nullptr};
    std::vector<kiss_fft_cpx> outputComplex;
};

FFT::FFT() = default;
FFT::~FFT() = default;

// allow move with default move constructor and move assignment operator
FFT::FFT(FFT&&) noexcept = default;
FFT& FFT::operator=(FFT&&) noexcept = default;

void FFT::initialise(size_t blockSizeTimeDomain) {
    impl_ = std::make_unique<Impl>(blockSizeTimeDomain);
}

std::span<const std::complex<float>> FFT::compute(std::span<const float> buffer) {
    auto& outputComplex = impl_->outputComplex;

    kiss_fftr(impl_->config, buffer.data(), outputComplex.data());

    // make sure the Kiss FFT output type can be casted to std::complex<float>
    static_assert(std::is_standard_layout_v<kiss_fft_cpx>);
    // check if real and imag types are floats
    static_assert(std::is_same_v<decltype(kiss_fft_cpx::r), float>);
    static_assert(std::is_same_v<decltype(kiss_fft_cpx::i), float>);
    // check if total size is equal (no padding)
    static_assert(sizeof(kiss_fft_cpx) == sizeof(std::complex<float>));

    return std::span(
        reinterpret_cast<const std::complex<float>*>(outputComplex.data()),
        outputComplex.size()
    );
}
