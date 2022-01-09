#pragma once

#include <charconv>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

class CliParser {
public:
    CliParser(int argc, char* argv[]) : args_(argv, argv + argc) {}

    auto args()    const noexcept { return args_; }
    auto nargs()   const noexcept { return args_.size(); }
    auto program() const noexcept {
        auto full = args_[0];
        // strip path
        for (auto&& sep : {"/", "\\"}) {
            if (auto pos = full.rfind(sep); pos != full.npos) {
                return full.substr(pos + 1);
            }
        }
        return full;
    }

    bool hasFlag(std::string_view flag) const {
        const auto it = std::find(args_.begin() + 1, args_.end(), flag);
        return it != args_.end();
    }

    std::optional<std::string_view> getValue(std::string_view option) const {
        auto it = std::find(args_.begin() + 1, args_.end(), option);
        if (it != args_.end()) ++it;
        if (it != args_.end()) return *it;
        return {};
    }

    template <typename T>
    std::optional<T> getValueAs(std::string_view option) const {
        const auto str = getValue(option);
        if (!str) return {};

        T result{};
        const auto [ptr, ec] { std::from_chars(str->data(), str->data() + str->size(), result) };
        if (ec != std::errc()) return {};
        return result;
    }

private:
    const std::vector<std::string_view> args_;
};

enum class Escape {
    Reset   = 0,
    Bold    = 1,
    Black   = 30,
    Red     = 31,
    Green   = 32,
    Yellow  = 33,
    Blue    = 34,
    Magenta = 35,
    Cyan    = 36,
    White   = 37,
};

std::ostream& operator<<(std::ostream& os, const Escape& code) {
    return os << "\033[" << static_cast<int>(code) << "m";
}

template<typename... Ts>
std::string concat(Ts const&... ts){
    std::stringstream s;
    (s << ... << ts);
    return s.str();
}

template <typename T>
std::string join(const T& c, const char* delimiter = ", ") {
    if (c.empty()) return {};
    std::stringstream ss;
    std::copy(
        c.begin(),
        std::prev(c.end()),
        std::ostream_iterator<typename T::value_type>(ss, delimiter)
    );
    ss << c.back();
    return ss.str();
}
