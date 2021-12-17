#pragma once

#include <array>
#include <cassert>
#include <span>
#include <vector>

#include <vamp/vamp.h>

#include "rt-vamp-plugin/Plugin.h"

namespace rtvamp {

class VampFeatureUnionWrapper {
public:
    VampFeatureUnionWrapper() {
        auto& v1 = getV1();
        v1.hasTimestamp = 0;
        v1.sec          = 0;
        v1.nsec         = 0;
        v1.valueCount   = 0;
        v1.values       = values_.data();
        v1.label        = nullptr;

        auto& v2 = getV2();
        v2.hasDuration  = 0;
        v2.durationSec  = 0;
        v2.durationNsec = 0;
    }

    size_t getValueCount() const noexcept { return values_.size(); }

    void setValueCount(size_t n) {
        values_.resize(n);
        auto& v1 = getV1();
        v1.valueCount = n;
        v1.values     = values_.empty() ? nullptr : values_.data();  // might got reallocated
    }

    void assignValues(const Feature& values) {
        if (const auto n = values.size(); getValueCount() != n) {
            setValueCount(n);
        }
        std::copy(values.begin(), values.end(), values_.begin());
    }

    VampFeatureUnion* get() noexcept { return featureUnion_.data(); }

private:
    VampFeature&   getV1() noexcept { return featureUnion_[0].v1; }
    VampFeatureV2& getV2() noexcept { return featureUnion_[1].v2; }

    std::vector<float>              values_;
    std::array<VampFeatureUnion, 2> featureUnion_;
};


class VampFeatureListsWrapper {
public:
    size_t getOutputCount() noexcept {
        assert(featureUnionWrappers_.size() == featureLists_.size());
        return featureLists_.size();
    }

    void setOutputCount(size_t n) {
        featureUnionWrappers_.resize(n);  // default construction
        featureLists_.clear();
        featureLists_.reserve(n);

        for (auto& wrapper : featureUnionWrappers_) {
            featureLists_.push_back({
                .featureCount = 1,
                .features     = wrapper.get()
            });
        }
    }

    void assignValues(size_t outputNumber, const Feature& values) {
        assert(outputNumber < getOutputCount());
        featureUnionWrappers_[outputNumber].assignValues(values);
    }

    void assignValues(const FeatureSet& values) {
        const auto outputCount = values.size();
        if (getOutputCount() != outputCount) {
            setOutputCount(outputCount);
        }
        for (size_t i = 0; i < outputCount; ++i) {
            assignValues(i, values[i]);
        }
    }

    VampFeatureList* get() noexcept { return featureLists_.data(); }

private:
    std::vector<VampFeatureUnionWrapper> featureUnionWrappers_;
    std::vector<VampFeatureList>         featureLists_;
};

}  // namespace rtvamp
