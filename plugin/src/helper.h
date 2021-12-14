#pragma once

#include <algorithm>
#include <string>
#include <vector>

namespace rtvamp {

inline void transformStringToConstChar(
    const std::vector<std::string>& vs, std::vector<const char*>& vc
) {
    vc.resize(vs.size());
    std::transform(
        vs.begin(), vs.end(), vc.begin(),
        [](const auto& s) { return s.c_str(); }
    );
}

}  // namespace rtvamp
