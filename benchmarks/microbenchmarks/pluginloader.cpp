#include <benchmark/benchmark.h>

#include "rtvamp/hostsdk/PluginLoader.hpp"

using rtvamp::hostsdk::PluginLoader;

static void BM_construct(benchmark::State& state) {
    for (auto _ : state) {
        PluginLoader loader;
        benchmark::DoNotOptimize(loader);
    }
}
BENCHMARK(BM_construct);

static void BM_getPluginPaths(benchmark::State& state) {
    for (auto _ : state) {
        const auto result = PluginLoader::getPluginPaths();
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_getPluginPaths);

static void BM_listLibraries(benchmark::State& state) {
    PluginLoader loader;
    for (auto _ : state) {
        const auto result = loader.listLibraries();
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_listLibraries);

static void BM_listPlugins(benchmark::State& state) {
    PluginLoader loader;
    for (auto _ : state) {
        const auto result = loader.listPlugins();
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_listPlugins);

static void BM_loadPlugin(benchmark::State& state) {
    PluginLoader loader;
    for (auto _ : state) {
        const auto plugin = loader.loadPlugin("example-plugin:rms", 48000);
        benchmark::DoNotOptimize(plugin);
    }
}
BENCHMARK(BM_loadPlugin);

BENCHMARK_MAIN();
