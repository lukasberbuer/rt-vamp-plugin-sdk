#include <algorithm>  // generate
#include <random>

#include <benchmark/benchmark.h>
#include <vamp-hostsdk/PluginHostAdapter.h>

#include "rtvamp/hostsdk/PluginHostAdapter.hpp"

#include "DescriptorRMS.hpp"

static void randomize(std::vector<float>& vec) {
    std::default_random_engine       engine;
    std::uniform_real_distribution<> dist(-1.0f, 1.0f);
    std::generate(vec.begin(), vec.end(), [&] { return dist(engine); });
}

constexpr auto descriptor = DescriptorRMS::get();

static void BM_rtvamp(benchmark::State& state) {
    rtvamp::hostsdk::PluginHostAdapter adapter(descriptor, 48000);

    const size_t       blockSize = state.range(0);
    std::vector<float> inputBuffer(blockSize);
    randomize(inputBuffer);

    adapter.initialise(blockSize, blockSize);

    for (auto _ : state) {
        const auto result = adapter.process(inputBuffer, 0);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_rtvamp)->RangeMultiplier(2)->Range(1 << 4, 1 << 16);

static void BM_vamp(benchmark::State& state) {
    Vamp::PluginHostAdapter adapter(&descriptor, 48000);

    const size_t              blockSize = state.range(0);
    std::vector<float>        inputBuffer(blockSize);
    std::vector<const float*> inputBuffers{inputBuffer.data()};
    randomize(inputBuffer);

    adapter.initialise(1, blockSize, blockSize);

    for (auto _ : state) {
        const auto result = adapter.process(inputBuffers.data(), Vamp::RealTime{});
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_vamp)->RangeMultiplier(2)->Range(1 << 4, 1 << 16);

BENCHMARK_MAIN();
