#pragma once

#include <algorithm>
#include <string>

namespace rtvamp::transform {

struct ToConstChar {
    const char* operator()(const std::string& s) { return s.c_str(); }
};

struct ToPtr {
    template <typename T>
    const T* operator()(const T& item) { return &item; }

    template <typename T>
    T* operator()(T& item) { return &item; }
};

template <typename T1, typename T2, typename Operation>
inline void all(const T1& input, T2& output, Operation op) {
    output.resize(input.size());
    std::transform(input.begin(), input.end(), output.begin(), op);
}

}  // namespace rtvamp::transform
