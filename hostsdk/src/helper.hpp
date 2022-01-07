#pragma once

#include <string>
#include <sstream>

namespace rtvamp::hostsdk::helper {

template<typename... Ts>
std::string concat(Ts const&... ts){
    std::stringstream s;
    (s << ... << ts);
    return s.str();
}

}  // namespace rtvamp::hostsdk::helper
