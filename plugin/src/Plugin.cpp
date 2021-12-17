#include "rt-vamp-plugin/Plugin.hpp"

namespace rtvamp {

void Plugin::initialiseFeatureSet() {
    const auto outputs     = getOutputDescriptors();
    const auto outputCount = outputs.size();

    auto& featureSet = getFeatureSet();
    featureSet.resize(outputCount);

    for (size_t i = 0; i < outputCount; ++i) {
        featureSet[i].resize(outputs[i].binCount);
    }
}

}  // namespace rtvamp
