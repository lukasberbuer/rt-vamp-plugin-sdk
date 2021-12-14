#include <string>
#include <vector>

#include "vamp/vamp.h"

#include "rt-vamp-plugin/Plugin.h"

#include "helper.h"

namespace rtvamp {

/**
 * Extended VampOutputDescriptor with RAII containers for easy cleanup.
 */
struct VampOutputDescriptorCpp : public VampOutputDescriptor {
    explicit VampOutputDescriptorCpp(const OutputDescriptor& d)
        : identifier_(d.identifier),
          name_(d.name),
          description_(d.description),
          unit_(d.unit),
          binNames_(d.binNames)
    {
        if (!binNames_.empty()) {
            binNames_.resize(d.binCount);  // crop or fill missing names with empty strings
        }
        transformStringToConstChar(binNames_, binNamesConstChar_);

        identifier       = identifier_.c_str();
        name             = name_.c_str();
        description      = description_.c_str();
        unit             = unit_.c_str();
        hasFixedBinCount = 1;
        binCount         = d.binCount;
        binNames         = binNames_.empty() ? nullptr : binNamesConstChar_.data();
        hasKnownExtents  = static_cast<int>(d.hasKnownExtents);
        minValue         = d.minValue;
        maxValue         = d.maxValue;
        isQuantized      = static_cast<int>(d.isQuantized);
        quantizeStep     = d.quantizeStep;
        sampleType       = vampOneSamplePerStep;
        sampleRate       = 0.0f;
        hasDuration      = 0;
    }

private:
    const std::string identifier_;
    const std::string name_;
    const std::string description_;
    const std::string unit_;
    std::vector<std::string> binNames_;
    std::vector<const char*> binNamesConstChar_;
};

}  // namespace rtvamp
