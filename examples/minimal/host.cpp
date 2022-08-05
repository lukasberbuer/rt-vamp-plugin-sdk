#include <algorithm>  // generate
#include <iostream>
#include <random>
#include <vector>

#include "rtvamp/hostsdk.hpp"

void randomize(std::vector<float>& vec) {
    std::default_random_engine       engine;
    std::uniform_real_distribution<> dist(-1.0f, 1.0f);
    std::generate(vec.begin(), vec.end(), [&] { return dist(engine); });
}

int main() {
    // list all plugins keys (library:plugin)
    for (auto&& key : rtvamp::hostsdk::listPlugins()) {
        std::cout << key.get() << std::endl;
    }

    const float  sampleRate = 48000.0f;
    const size_t blockSize  = 4096;
    const size_t stepSize   = 4096;

    auto plugin = rtvamp::hostsdk::loadPlugin("minimal-plugin:zerocrossing", sampleRate);

    plugin->initialise(stepSize, blockSize);

    uint64_t           timestampNanoseconds = 0;
    std::vector<float> buffer(blockSize);

    randomize(buffer);

    auto features = plugin->process(buffer, timestampNanoseconds);

    std::cout << "Zero crossings: " << features[0][0] << std::endl;

    return 0;
}
