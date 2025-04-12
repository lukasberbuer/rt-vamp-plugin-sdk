#pragma once

#include <functional>
#include <string>
#include <sstream>
#include <type_traits>

namespace rtvamp::hostsdk::helper {

template<typename... Ts>
std::string concat(Ts const&... ts){
    std::stringstream s;
    (s << ... << ts);
    return s.str();
}

/**
 * General-purpose scope guard intended to call its exit function when a scope is exited.
 * @see https://en.cppreference.com/w/cpp/experimental/scope_exit
 */
template <typename Fn>
class [[nodiscard]] ScopeExit {
public:
    static_assert(std::is_nothrow_move_constructible_v<Fn>);
    static_assert(std::is_invocable_v<Fn>);

    explicit ScopeExit(Fn&& fn) noexcept
        : fn_(std::move(fn)) {}

    ScopeExit(ScopeExit&& other) noexcept
        : fn_(std::move(other.fn_)),
          active_(other.active_) {
        other.release();
    }

    ~ScopeExit() noexcept(std::is_nothrow_invocable_v<Fn>) {
        if (active_) {
            std::invoke(fn_);
        }
    }

    ScopeExit(const ScopeExit&) = delete;
    ScopeExit& operator=(const ScopeExit&) = delete;
    ScopeExit& operator=(ScopeExit&&) = delete;

    void release() noexcept {
        active_ = false;
    }

private:
    Fn fn_;
    bool active_{true};
};

}  // namespace rtvamp::hostsdk::helper
