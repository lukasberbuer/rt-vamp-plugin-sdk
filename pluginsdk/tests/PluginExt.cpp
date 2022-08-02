#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "rtvamp/pluginsdk.hpp"

using Catch::Matchers::Equals;

class TestPluginExt : public rtvamp::pluginsdk::PluginExt<TestPluginExt, 0> {
public:
    using PluginExt::PluginExt;  // inherit constructor

    static constexpr Meta meta{};

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
            .minValue     = -100.0f,
            .maxValue     = 100.0f,
            .quantizeStep = 1.0f,
        },
    };

    static constexpr std::array programs{"default", "new"};

    OutputList getOutputDescriptors() const override {
        return {};
    }

    void reset() override {};

    bool initialise(uint32_t stepSize, uint32_t blockSize) override {
        initialiseFeatureSet();
        return true;
    };

    const FeatureSet& process(InputBuffer buffer, uint64_t nsec) override {
        return getFeatureSet();
    };

    // to check callback functionality
    std::string onParameterChangeId{};
    float       onParameterChangeValue{};
    std::string onProgramChangeName{};

    void onParameterChange(std::string_view id, float newValue) override {
        onParameterChangeId    = id;
        onParameterChangeValue = newValue;
    }

    void onProgramChange(std::string_view newProgram) override {
        onProgramChangeName = newProgram;
    }
};

TEST_CASE("PluginExt get/set parameter") {
    TestPluginExt plugin(48000);

    SECTION("Default values") {
        REQUIRE(plugin.getParameter("limited").value() == 10.0f);
        REQUIRE(plugin.getParameter("quantized").value() == 1.0f);
        REQUIRE_FALSE(plugin.getParameter("invalid").has_value());
    }

    SECTION("Set/get") {
        REQUIRE(plugin.setParameter("limited", 1.1f));
        REQUIRE(plugin.getParameter("limited").value() == 1.1f);

        REQUIRE(plugin.setParameter("quantized", 10.0f));
        REQUIRE(plugin.getParameter("quantized").value() == 10.0f);

        REQUIRE_FALSE(plugin.setParameter("invalid", 0.0f));
    }

    SECTION("Clamp to limits") {
        REQUIRE(plugin.setParameter("limited", 1e9f));
        REQUIRE(plugin.getParameter("limited").value() == 10.0f);

        REQUIRE(plugin.setParameter("limited", -11.f));
        REQUIRE(plugin.getParameter("limited").value() == -10.0f);
    }

    SECTION("Quantization") {
        REQUIRE(plugin.setParameter("quantized", 0.0f));
        REQUIRE(plugin.getParameter("quantized").value() == 0.0f);

        REQUIRE(plugin.setParameter("quantized", 1.1f));
        REQUIRE(plugin.getParameter("quantized").value() == 1.0f);

        REQUIRE(plugin.setParameter("quantized", -1.9f));
        REQUIRE(plugin.getParameter("quantized").value() == -2.0f);

        REQUIRE(plugin.setParameter("quantized", 55.5f));
        REQUIRE(plugin.getParameter("quantized").value() == 56.0f);
    }
}

TEST_CASE("PluginExt get/select program") {
    TestPluginExt plugin(48000);

    // convert to string for Catch2 matchers
    const auto currentProgram = [&] { return std::string(plugin.getCurrentProgram()); };

    REQUIRE_THAT(currentProgram(), Equals("default"));

    REQUIRE_FALSE(plugin.selectProgram("invalid"));
    REQUIRE_THAT(currentProgram(), Equals("default"));

    REQUIRE(plugin.selectProgram("new"));
    REQUIRE_THAT(currentProgram(), Equals("new"));
}

TEST_CASE("PluginExt onParameterChange callback") {
    TestPluginExt plugin(48000);

    SECTION("Check defaults") {
        CHECK(plugin.onParameterChangeId.empty());
        CHECK(plugin.onParameterChangeValue == 0.0f);
    }

    SECTION("Invalid parameter input should not trigger callback") {
        plugin.setParameter("invalid", 11.11f);
        CHECK(plugin.onParameterChangeId.empty());
        CHECK(plugin.onParameterChangeValue == 0.0f);
    }

    SECTION("Valid parameter input") {
        plugin.setParameter("limited", 1.1f);
        CHECK(plugin.onParameterChangeId == "limited");
        CHECK(plugin.onParameterChangeValue == 1.1f);
    }
}

TEST_CASE("PluginExt onProgramChange callback") {
    TestPluginExt plugin(48000);

    SECTION("Check defaults") {
        CHECK(plugin.onProgramChangeName.empty());
    }

    SECTION("Invalid program selection should not trigger callback") {
        plugin.selectProgram("invalid");
        CHECK(plugin.onProgramChangeName.empty());
    }

    SECTION("Valid program selection") {
        plugin.selectProgram("new");
        CHECK(plugin.onProgramChangeName == "new");
    }
}

TEST_CASE("PluginExt within PluginAdapter") {
    using rtvamp::pluginsdk::detail::PluginAdapter;

    REQUIRE(PluginAdapter<TestPluginExt>::getDescriptor() != nullptr);
}
