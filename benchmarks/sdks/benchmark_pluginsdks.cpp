#include <benchmark/benchmark.h>
#include <vamp-sdk/PluginAdapter.h>

#include "rtvamp/pluginsdk.hpp"

#include "helper.hpp"
#include "RMS.hpp"

static void BM_plugin(benchmark::State& state, const VampPluginDescriptor* descriptor) {
    auto* handle = descriptor->instantiate(descriptor, 48000);

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

    addRateCounter(state, blockSize);
}

static void BM_rtvamp(benchmark::State& state) {
    constexpr auto* descriptor = rtvamp::pluginsdk::detail::PluginAdapter<RMS>::getDescriptor();
    BM_plugin(state, descriptor);
}
BENCHMARK(BM_rtvamp)->RangeMultiplier(2)->Range(1 << 4, 1 << 16);
BENCHMARK(BM_rtvamp)->Args({4096})->Threads(1)->UseRealTime();
BENCHMARK(BM_rtvamp)->Args({4096})->Threads(2)->UseRealTime();
BENCHMARK(BM_rtvamp)->Args({4096})->Threads(4)->UseRealTime();
BENCHMARK(BM_rtvamp)->Args({4096})->Threads(8)->UseRealTime();
BENCHMARK(BM_rtvamp)->Args({4096})->Threads(12)->UseRealTime();
BENCHMARK(BM_rtvamp)->Args({4096})->Threads(16)->UseRealTime();

static void BM_vamp(benchmark::State& state) {
    static Vamp::PluginAdapter<RMSvamp> adapter;
    BM_plugin(state, adapter.getDescriptor());
}
BENCHMARK(BM_vamp)->RangeMultiplier(2)->Range(1 << 4, 1 << 16);
BENCHMARK(BM_vamp)->Args({4096})->Threads(1)->UseRealTime();
BENCHMARK(BM_vamp)->Args({4096})->Threads(2)->UseRealTime();
BENCHMARK(BM_vamp)->Args({4096})->Threads(4)->UseRealTime();
BENCHMARK(BM_vamp)->Args({4096})->Threads(8)->UseRealTime();
BENCHMARK(BM_vamp)->Args({4096})->Threads(12)->UseRealTime();
BENCHMARK(BM_vamp)->Args({4096})->Threads(16)->UseRealTime();

BENCHMARK_MAIN();
