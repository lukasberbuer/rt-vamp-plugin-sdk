#include <benchmark/benchmark.h>

#include "rtvamp/pluginsdk.hpp"

class TestPluginExt : public rtvamp::pluginsdk::PluginExt<TestPluginExt, 0> {
public:
    using PluginExt::PluginExt;  // inherit constructor

    static constexpr Meta       meta{};
    static constexpr std::array parameters{
        ParameterDescriptor{
            .identifier   = "limited",
            .name         = "Parameter with limits",
            .description  = "",
            .unit         = "",
            .defaultValue = 10.0f,
            .minValue     = -10.0f,
            .maxValue     = 10.0f,
            .quantizeStep = std::nullopt,
        },
        ParameterDescriptor{
            .identifier   = "quantized",
            .name         = "Parameter with quantization",
            .description  = "",
            .unit         = "",
            .defaultValue = 1.0f,
            .minValue     = 0.0f,
            .maxValue     = 100.0f,
            .quantizeStep = 1.0f,
        },
    };

    OutputList getOutputDescriptors() const override { return {}; }

    void reset() override {};

    bool initialise(uint32_t stepSize, uint32_t blockSize) override {
        initialiseFeatureSet();
        return true;
    };

    const FeatureSet& process(InputBuffer buffer, uint64_t nsec) override {
        return getFeatureSet();
    };
};


static void BM_getParameter(benchmark::State& state) {
    TestPluginExt plugin(48000);
    for (auto _ : state) {
        const auto value = plugin.getParameter("limited");
        benchmark::DoNotOptimize(value);
    }
}
BENCHMARK(BM_getParameter);

static void BM_setParameter(benchmark::State& state, std::string_view id) {
    TestPluginExt plugin(48000);
    for (auto _ : state) {
        plugin.setParameter(id, 11.11f);
    }
}
BENCHMARK_CAPTURE(BM_setParameter, limited, "limited");
BENCHMARK_CAPTURE(BM_setParameter, quantized, "quantized");

BENCHMARK_MAIN();
