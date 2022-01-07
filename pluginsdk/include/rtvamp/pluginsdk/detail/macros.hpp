#pragma once

#include <iostream>
#include <ostream>

namespace rtvamp::pluginsdk::detail {

template <typename Arg, typename... Args>
void print(std::ostream& out, Arg&& arg, Args&&... args) {
    out << std::forward<Arg>(arg);
    ((out << std::forward<Args>(args)), ...);
    out << std::endl;
}

}  // namespace rtvamp::pluginsdk::detail

#ifdef NDEBUG
#define RTVAMP_DEBUG(...)
#else
#define RTVAMP_DEBUG(...) ::rtvamp::pluginsdk::detail::print(std::cerr, "[DEBUG] ", __VA_ARGS__);
#endif

#define RTVAMP_ERROR(...) ::rtvamp::pluginsdk::detail::print(std::cerr, "[ERROR] ", __VA_ARGS__);
