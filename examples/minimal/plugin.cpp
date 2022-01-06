#include "rtvamp/pluginsdk/EntryPoint.hpp"
#include "rtvamp/pluginsdk/PluginDefinition.hpp"

class ZeroCrossing : public rtvamp::pluginsdk::PluginDefinition<1 /* one output */> {
public:
    using PluginDefinition::PluginDefinition;  // inherit constructor

    static constexpr Meta meta{
        .identifier    = "zerocrossing",
        .name          = "Zero crossings",
        .description   = "Detect and count zero crossings",
        .maker         = "LB",
        .copyright     = "MIT",
        .pluginVersion = 1,
        .inputDomain   = InputDomain::Time,
    };

    OutputList getOutputDescriptors() const override {
        return {
            OutputDescriptor{
                .identifier  = "counts",
                .name        = "Zero crossing counts",
                .description = "The number of zero crossing points per processing block",
                .unit        = "",
                .binCount    = 1,
            },
        };
    }

    bool initialise(uint32_t stepSize, uint32_t blockSize) override {
        initialiseFeatureSet();  // automatically resizes feature set to number of outputs and bins
        return true;
    };

    void reset() override {
        previousSample_ = 0.0f;
    }

    FeatureSet process(InputBuffer buffer, uint64_t nsec) override {
        size_t crossings   = 0;
        bool   wasPositive = (previousSample_ >= 0.0f);

        for (const auto& sample : std::get<TimeDomainBuffer>(buffer)) {
            const bool isPositive = (sample >= 0.0f);
            crossings += int(isPositive != wasPositive);
            wasPositive = isPositive;
        }

        auto& result = getFeatureSet();
        result[0][0] = crossings;  // first and only output, first and only bin
        return result;             // return and span/view of the results
    };

private:
    float previousSample_ = 0.0f;
};

RTVAMP_ENTRY_POINT(ZeroCrossing)
