#include <algorithm>  // generate
#include <random>

#include <benchmark/benchmark.h>
#include <vamp-sdk/PluginAdapter.h>

#include "rtvamp/pluginsdk/PluginAdapter.hpp"

#include "RMS.hpp"

static void randomize(std::vector<float>& vec) {
    std::default_random_engine       engine;
    std::uniform_real_distribution<> dist(-1.0f, 1.0f);
    std::generate(vec.begin(), vec.end(), [&] { return dist(engine); });
}

template <typename TPluginAdapter>
static void BM_plugin(benchmark::State& state) {
    TPluginAdapter adapter;
    const auto*    descriptor = adapter.getDescriptor();
    auto*          handle     = descriptor->instantiate(descriptor, 48000);

    const size_t              blockSize = state.range(0);
    std::vector<float>        inputBuffer(blockSize);
    std::vector<const float*> inputBuffers{inputBuffer.data()};
    randomize(inputBuffer);

    descriptor->initialise(handle, 1, blockSize, blockSize);

    for (auto _ : state) {
        auto* result = descriptor->process(handle, inputBuffers.data(), 0, 0);
        benchmark::DoNotOptimize(result);
        descriptor->releaseFeatureSet(result);
    }

    descriptor->cleanup(handle);
}
// single-threaded with block size variation
BENCHMARK_TEMPLATE(BM_plugin, rtvamp::pluginsdk::PluginAdapter<RMS>)
    ->RangeMultiplier(2)->Range(1 << 4, 1 << 16);
BENCHMARK_TEMPLATE(BM_plugin, Vamp::PluginAdapter<RMSvamp>)
    ->RangeMultiplier(2)->Range(1 << 4, 1 << 16);
// multi-threaded with fixed block size
BENCHMARK_TEMPLATE(BM_plugin, rtvamp::pluginsdk::PluginAdapter<RMS>)
    ->Args({4096})->Threads(4)->UseRealTime();
BENCHMARK_TEMPLATE(BM_plugin, Vamp::PluginAdapter<RMSvamp>)
    ->Args({4096})->Threads(4)->UseRealTime();

BENCHMARK_MAIN();
