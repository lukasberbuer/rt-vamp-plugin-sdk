#include <benchmark/benchmark.h>
#include <vamp-hostsdk/PluginHostAdapter.h>

#include "rtvamp/hostsdk/PluginHostAdapter.hpp"

#include "helper.hpp"

// Vamp prevents includes from both sdk and hostsdk, use external linkage instead
extern VampPluginDescriptor* getVampDescriptor();
extern VampPluginDescriptor* getRtvampDescriptor();

static void BM_rtvamp(benchmark::State& state) {
    const auto* descriptor = getRtvampDescriptor();
    rtvamp::hostsdk::PluginHostAdapter adapter(*descriptor, 48000);

    const size_t       blockSize = state.range(0);
    std::vector<float> inputBuffer(blockSize);
    randomize(inputBuffer);

    adapter.initialise(blockSize, blockSize);

    for (auto _ : state) {
        const auto result = adapter.process(inputBuffer, 0);
        benchmark::DoNotOptimize(result);
    }
    addRateCounter(state, blockSize);
}
BENCHMARK(BM_rtvamp)->RangeMultiplier(2)->Range(1 << 4, 1 << 16);
BENCHMARK(BM_rtvamp)->Args({4096})->Threads(1)->UseRealTime();
BENCHMARK(BM_rtvamp)->Args({4096})->Threads(2)->UseRealTime();
BENCHMARK(BM_rtvamp)->Args({4096})->Threads(4)->UseRealTime();
BENCHMARK(BM_rtvamp)->Args({4096})->Threads(8)->UseRealTime();
BENCHMARK(BM_rtvamp)->Args({4096})->Threads(12)->UseRealTime();
BENCHMARK(BM_rtvamp)->Args({4096})->Threads(16)->UseRealTime();

static void BM_vamp(benchmark::State& state) {
    const auto* descriptor = getVampDescriptor();
    Vamp::PluginHostAdapter adapter(descriptor, 48000);

    const size_t              blockSize = state.range(0);
    std::vector<float>        inputBuffer(blockSize);
    std::vector<const float*> inputBuffers{inputBuffer.data()};
    randomize(inputBuffer);

    adapter.initialise(1, blockSize, blockSize);

    for (auto _ : state) {
        const auto result = adapter.process(inputBuffers.data(), Vamp::RealTime{});
        benchmark::DoNotOptimize(result);
    }
    addRateCounter(state, blockSize);
}
BENCHMARK(BM_vamp)->RangeMultiplier(2)->Range(1 << 4, 1 << 16);
BENCHMARK(BM_vamp)->Args({4096})->Threads(1)->UseRealTime();
BENCHMARK(BM_vamp)->Args({4096})->Threads(2)->UseRealTime();
BENCHMARK(BM_vamp)->Args({4096})->Threads(4)->UseRealTime();
BENCHMARK(BM_vamp)->Args({4096})->Threads(8)->UseRealTime();
BENCHMARK(BM_vamp)->Args({4096})->Threads(12)->UseRealTime();
BENCHMARK(BM_vamp)->Args({4096})->Threads(16)->UseRealTime();

BENCHMARK_MAIN();
