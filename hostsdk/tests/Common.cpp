#include <catch2/catch_test_macros.hpp>

#include <cstdlib>

TEST_CASE("VAMP_PATH environment variable") {
    INFO("VAMP_PATH environment variable must be set to output directory to discover test plugins");
    REQUIRE(std::getenv("VAMP_PATH"));
}
