#pragma once

#include <complex>
#include <memory>
#include <span>
#include <vector>

std::vector<float> hanning(size_t length);
std::vector<float> hamming(size_t length);

class FFT {
public:
    FFT();
    ~FFT();

    FFT(const FFT&) = delete;
    FFT(FFT&&) noexcept;

    FFT& operator=(const FFT&) = delete;
    FFT& operator=(FFT&&) noexcept;

    void initialise(size_t blockSizeTimeDomain);

    std::span<const std::complex<float>> compute(std::span<const float> buffer);

private:
    struct Impl;

    std::unique_ptr<Impl> impl_;
};
