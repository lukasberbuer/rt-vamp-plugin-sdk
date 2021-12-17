# Real-time Vamp plugin SDK for C++20

Vamp is an C/C++ plugin API for audio analysis / feature extraction plugins: https://www.vamp-plugins.org

This SDK targets performance-critical applications by:

- reducing memory allocations
- restricting and simplifying the plugin API

The SDK aims to be **well tested**, **cross-platform** and use **modern C++**.

```
rt-vamp-plugin-sdk

rt-vamp-plugin/
rt-vamp-host/
```

## Why another SDK?

The [official SDK](https://github.com/c4dm/vamp-plugin-sdk) offers a convinient [C++ plugin API](https://code.soundsoftware.ac.uk/projects/vamp-plugin-sdk/embedded/classVamp_1_1Plugin.html).
But there are some drawbacks for real-time processing:

- Huge amount of memory allocations due to the extensive use of C++ containers like vectors and lists **passed by value**.

  Let's have a look at the `process` function which does the main work:

  `FeatureSet process(const float *const *inputBuffers, RealTime timestamp)`

  `FeatureSet` is returned by value and is a `std::map<int, FeatureList>`.
  `FeatureList` is a `std::vector<Feature>` and `Feature` is `struct` containing the actual feature values as a `std::vector<float>`.
  So in total, those are three nested containers, which are all heap allocated.

- The C++ API is a wrapper of the C API.

  On the plugin side, the `PluginAdapter` class converts the C++ containers to C level ([code](https://github.com/c4dm/vamp-plugin-sdk/blob/master/src/vamp-sdk/PluginAdapter.cpp#L828-L921)).
  Therefore the C++ containers are temporary objects and will be deallocated shortly after creation.

  On the host side, the `PluginHostAdapter` converts again from the C to the C++ representation ([code](https://github.com/c4dm/vamp-plugin-sdk/blob/master/src/vamp-hostsdk/PluginHostAdapter.cpp#L413-L464)).

## Restrictions

Following features of the Vamp API are restricted by the `rt-vamp-plugin-sdk`:

- `hasFixedBinCount` has to be `true`. The number of values is constant for each feature during processing.
  This has the advantage, that the memory for the feature vector can be preallocated.

- `SampleType` has to be `OneSamplePerStep`: the plugin will generate one feature set for each input block.
  
  Following parameters are therefore negitable:
  - `Vamp::Plugin::OutputDescriptor::sampleRate`
  - `Vamp::Plugin::OutputDescriptor::hasDuration`
  - `Vamp::Plugin::Feature::hasTimestamp` / `Vamp::Plugin::Feature::timestamp`
  - `Vamp::Plugin::Feature::hasDuration` / `Vamp::Plugin::Feature::duration`

- Only one input channel allowed
