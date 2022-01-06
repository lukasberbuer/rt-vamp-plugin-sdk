# Real-time Vamp plugin SDK for C++20

Vamp is an C/C++ plugin API for audio analysis / feature extraction plugins: https://www.vamp-plugins.org

This SDK for plugins and hosts targets performance-critical applications by:

- reducing memory allocations, **no memory allocation** during processing
- simplifying and restricting the plugin API
- `constexpr` evaluation for compile-time errors instead of runtime errors

The SDK aims to be **well tested**, **cross-platform** and use **modern C++**.

Compiler support:

- GCC >= 10
- Clang >= 11
- MSVC >= 19.30

TODOs:

- handle initialise false return value in hostsdk
- mixins for parameters in pluginsdk
- FFT in hostsdk
- multi-channel wrapper for hostsdk
- more example plugins
- benchmark analysis script?

Further ideas:

- Plugin tester as library and executable
- Python bindings for host
- Adapter library for Python plugins?
- Adapter library for WASM plugins?

## Minimal example

```cpp
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

    void reset() override { previousSample_ = 0.0f; }

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
```

## Vamp ecosystem

- [Great collection of plugins](https://www.vamp-plugins.org/download.html)
- [Sonic Visualiser](https://www.sonicvisualiser.org/): Open-source software to visualize, analyze and annotate audio
- [Sonic Annotator](https://vamp-plugins.org/sonic-annotator): Batch tool for feature extraction
- [Audacity supports Vamp plugins](https://wiki.audacityteam.org/wiki/Vamp_Plug-ins)

## Why another SDK?

The [official SDK](https://github.com/c4dm/vamp-plugin-sdk) offers a convenient [C++ plugin API](https://code.soundsoftware.ac.uk/projects/vamp-plugin-sdk/embedded/classVamp_1_1Plugin.html).
But there are some drawbacks for real-time processing:

- Huge amount of memory allocations due to the extensive use of C++ containers like vectors and lists **passed by value**.

  Let's have a look at the `process` method of the `Vamp::Plugin` class which does the main work:

  `FeatureSet process(const float *const *inputBuffers, RealTime timestamp)`

  `FeatureSet` is returned by value and is a `std::map<int, FeatureList>`.
  `FeatureList` is a `std::vector<Feature>` and `Feature` is `struct` containing the actual feature values as a `std::vector<float>`.
  So in total, those are three nested containers, which are all heap allocated.

- The C++ API is a wrapper of the C API.

  On the plugin side, the `PluginAdapter` class converts the C++ containers to C level ([code](https://github.com/c4dm/vamp-plugin-sdk/blob/master/src/vamp-sdk/PluginAdapter.cpp#L828-L921)).
  Therefore the C++ containers are temporary objects and will be deallocated shortly after creation.

  On the host side, the `PluginHostAdapter` converts again from the C to the C++ representation ([code](https://github.com/c4dm/vamp-plugin-sdk/blob/master/src/vamp-hostsdk/PluginHostAdapter.cpp#L413-L464)).

## Plugin restrictions

Following features of the Vamp API `Vamp::Plugin` are restricted:

- `OutputDescriptor::hasFixedBinCount == true` for every output.
  The number of values is constant for each feature during processing.
  This has the advantage, that memory for the feature vector can be preallocated.

- `OutputDescriptor::SampleType == OneSamplePerStep` for every output.
  The plugin will generate one feature set for each input block.
  
  Following parameters are therefore negitable:
  - `OutputDescriptor::sampleRate`
  - `OutputDescriptor::hasDuration`
  - `Feature::hasTimestamp` & `Feature::timestamp`
  - `Feature::hasDuration` & `Feature::duration`

- Only one input channel allowed: `getMinChannelCount() == 1`
