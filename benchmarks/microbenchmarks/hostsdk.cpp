#include <benchmark/benchmark.h>

#include "rtvamp/hostsdk.hpp"

static void BM_getVampPaths(benchmark::State& state) {
    for (auto _ : state) {
        const auto libraries = rtvamp::hostsdk::getVampPaths();
        benchmark::DoNotOptimize(libraries);
    }
}
BENCHMARK(BM_getVampPaths);

static void BM_listLibraries(benchmark::State& state) {
    for (auto _ : state) {
        const auto libraries = rtvamp::hostsdk::listLibraries();
        benchmark::DoNotOptimize(libraries);
    }
}
BENCHMARK(BM_listLibraries);

static void BM_listPlugins(benchmark::State& state) {
    for (auto _ : state) {
        const auto plugins = rtvamp::hostsdk::listPlugins();
        benchmark::DoNotOptimize(plugins);
    }
}
BENCHMARK(BM_listPlugins);

static void BM_listPluginsInLibrary(benchmark::State& state) {
    const auto libraries = rtvamp::hostsdk::listLibraries();
    for (auto _ : state) {
        for (auto&& library : libraries) {
            const auto plugins = rtvamp::hostsdk::listPlugins(library);
            benchmark::DoNotOptimize(plugins);
        }
    }
}
BENCHMARK(BM_listPluginsInLibrary);

static void BM_listPluginsInLibraries(benchmark::State& state) {
    const auto libraries = rtvamp::hostsdk::listLibraries();
    for (auto _ : state) {
        const auto plugins = rtvamp::hostsdk::listPlugins(libraries);
        benchmark::DoNotOptimize(plugins);
    }
}
BENCHMARK(BM_listPluginsInLibraries);

static void BM_loadAllPlugins(benchmark::State& state) {
    const auto plugins = rtvamp::hostsdk::listPlugins();
    for (auto _ : state) {
        for (auto&& key : plugins) {
            try {
                const auto plugin = rtvamp::hostsdk::loadPlugin(key, 48000);
                benchmark::DoNotOptimize(plugin);
            } catch (...) {}
        }
    }
}
BENCHMARK(BM_loadAllPlugins);

static void BM_loadAllPluginsCachedLibraryPaths(benchmark::State& state) {
    const auto libraries = rtvamp::hostsdk::listLibraries();
    const auto plugins   = rtvamp::hostsdk::listPlugins(libraries);
    for (auto _ : state) {
        for (auto&& key : plugins) {
            try {
                const auto plugin = rtvamp::hostsdk::loadPlugin(key, 48000, libraries);
                benchmark::DoNotOptimize(plugin);
            } catch (...) {}
        }
    }
}
BENCHMARK(BM_loadAllPluginsCachedLibraryPaths);

BENCHMARK_MAIN();
