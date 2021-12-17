#pragma once

#include <string>
#include <vector>

#include "vamp/vamp.h"

#include "rt-vamp-plugin/Plugin.hpp"

#include "helper.hpp"

namespace rtvamp {

/**
 * Extended VampOutputDescriptor with RAII containers for easy cleanup.
 */
struct VampOutputDescriptorWrapper{
    explicit VampOutputDescriptorWrapper(const OutputDescriptor& d)
        : identifier_(d.identifier),
          name_(d.name),
          description_(d.description),
          unit_(d.unit),
          binNames_(d.binNames)
    {
        if (!binNames_.empty()) {
            binNames_.resize(d.binCount);  // crop or fill missing names with empty strings
        }
        transform::all(binNames_, binNamesConstChar_, transform::ToConstChar{});

        descriptor_.identifier       = identifier_.c_str();
        descriptor_.name             = name_.c_str();
        descriptor_.description      = description_.c_str();
        descriptor_.unit             = unit_.c_str();
        descriptor_.hasFixedBinCount = 1;
        descriptor_.binCount         = d.binCount;
        descriptor_.binNames         = binNames_.empty() ? nullptr : binNamesConstChar_.data();
        descriptor_.hasKnownExtents  = static_cast<int>(d.hasKnownExtents);
        descriptor_.minValue         = d.minValue;
        descriptor_.maxValue         = d.maxValue;
        descriptor_.isQuantized      = static_cast<int>(d.isQuantized);
        descriptor_.quantizeStep     = d.quantizeStep;
        descriptor_.sampleType       = vampOneSamplePerStep;
        descriptor_.sampleRate       = 0.0f;
        descriptor_.hasDuration      = 0;
    }

    const VampOutputDescriptor& get() const noexcept { return descriptor_; }
    VampOutputDescriptor&       get()       noexcept { return descriptor_; }

private:
    VampOutputDescriptor descriptor_;

    const std::string identifier_;
    const std::string name_;
    const std::string description_;
    const std::string unit_;

    std::vector<std::string> binNames_;
    std::vector<const char*> binNamesConstChar_;
};

}  // namespace rtvamp
