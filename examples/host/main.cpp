#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <sndfile.hh>

#include "rtvamp/hostsdk.hpp"

#include "FFT.hpp"
#include "helper.hpp"

using rtvamp::hostsdk::Plugin;

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::optional<T>& opt) {
    if (!opt) return os << "null";
    return os << opt.value();
}

std::ostream& operator<<(std::ostream& os, const Plugin::InputDomain& d) {
    return os << (d == Plugin::InputDomain::Frequency ? "Frequency" : "Time");
}

void listPlugins() {
    for (auto&& lib : rtvamp::hostsdk::listLibraries()) {
        for (auto&& key : rtvamp::hostsdk::listPlugins(lib)) {
            std::cout << std::boolalpha;
            std::cout << Escape::Blue << "Plugin " << Escape::Bold << key.get() << Escape::Reset 
                << " (" << lib.string() << ")\n";

            try {
                const auto plugin = rtvamp::hostsdk::loadPlugin(key, 48000);
                std::cout << "- Identifier:           " << plugin->getIdentifier() << '\n';
                std::cout << "- Name:                 " << plugin->getName() << '\n';
                std::cout << "- Description:          " << plugin->getDescription() << '\n';
                std::cout << "- Maker:                " << plugin->getMaker() << '\n';
                std::cout << "- Copyright:            " << plugin->getCopyright() << '\n';
                std::cout << "- Plugin version:       " << plugin->getPluginVersion() << '\n';
                std::cout << "- Input domain:         " << plugin->getInputDomain() << '\n';
                std::cout << "- Preferred step size:  " << plugin->getPreferredStepSize() << '\n';
                std::cout << "- Preferred block size: " << plugin->getPreferredBlockSize() << '\n';
                std::cout << "- Programs:             " << join(plugin->getPrograms()) << '\n';

                std::cout << "- Parameters:\n";
                size_t parameterIndex = 0;
                for (auto&& p : plugin->getParameterDescriptors()) {
                    std::cout << "  - Parameter " << ++parameterIndex << ":\n";
                    std::cout << "    - Identifier:       " << p.identifier << '\n';
                    std::cout << "    - Name:             " << p.name << '\n';
                    std::cout << "    - Description:      " << p.description << '\n';
                    std::cout << "    - Unit:             " << p.unit << '\n';
                    std::cout << "    - Default value:    " << p.defaultValue << '\n';
                    std::cout << "    - Minimum value:    " << p.minValue << '\n';
                    std::cout << "    - Maximum value:    " << p.maxValue << '\n';
                    std::cout << "    - Quantize step:    " << p.quantizeStep << '\n';
                }

                std::cout << "- Outputs:\n";
                size_t outputIndex = 0;
                for (auto&& o : plugin->getOutputDescriptors()) {
                    std::cout << "  - Output " << ++outputIndex << ":\n";
                    std::cout << "    - Identifier:       " << o.identifier << '\n';
                    std::cout << "    - Name:             " << o.name << '\n';
                    std::cout << "    - Description:      " << o.description << '\n';
                    std::cout << "    - Unit:             " << o.unit << '\n';
                    std::cout << "    - Bin count:        " << o.binCount << '\n';
                    std::cout << "    - Bin names:        " << join(o.binNames) << '\n';
                    std::cout << "    - Known extends:    " << o.hasKnownExtents << '\n';
                    std::cout << "    - Minimum value:    " << o.minValue << '\n';
                    std::cout << "    - Maximum value:    " << o.maxValue << '\n';
                    std::cout << "    - Quantize step:    " << o.quantizeStep << '\n';
                }
            } catch (const std::exception& e) {
                std::cout << Escape::Red << "[ERROR] " << e.what() << Escape::Reset << '\n';
            }
            std::cout << '\n';
        }
    }
    std::cout << std::flush;
}

void listPaths() {
    for (auto&& path : rtvamp::hostsdk::getVampPaths()) {
        std::cout << path.string() << std::endl;
    }
}

void listPluginIds() {
    for (auto&& key : rtvamp::hostsdk::listPlugins()) {
        std::cout << key.get() << std::endl;
    }
}

void listPluginOutputs() {
    for (auto&& key : rtvamp::hostsdk::listPlugins()) {
        try {
            auto plugin = rtvamp::hostsdk::loadPlugin(key, 48000);
            for (auto&& output : plugin->getOutputDescriptors()) {
                std::cout << key.get() << ':' << output.identifier << std::endl;
            }
        } catch (...) {}
    }
}

void process(
    std::string_view        pluginKey,
    std::string_view        audiofile,
    std::optional<uint32_t> optionalOutputIndex,
    std::optional<uint32_t> optionalBlockSize
) {
    // load audio file
    auto file = SndfileHandle(std::string(audiofile));
    if (file.error()) {
        throw std::runtime_error(
            concat("Failed to open audio file: ", audiofile, " (", file.strError(), ")")
        );
    }

    const auto sampleRate = file.samplerate(); 
    const auto channels   = file.channels();

    // load plugin
    auto plugin = rtvamp::hostsdk::loadPlugin(pluginKey, sampleRate);

    const auto getBlockSize = [&]() -> uint32_t {
        if (optionalBlockSize) {
            return optionalBlockSize.value();
        }
        if (const auto preferredBlockSize = plugin->getPreferredBlockSize()) {
            return preferredBlockSize;
        }
        return 1024;
    };

    const uint32_t blockSize = getBlockSize();
    const uint32_t stepSize  = blockSize;

    const bool success = plugin->initialise(stepSize, blockSize);
    if (!success) throw std::runtime_error("Initialisation failed");

    const auto outputIndex = optionalOutputIndex.value_or(0);
    const auto output = [&] {
        try {
            return plugin->getOutputDescriptors().at(outputIndex);
        } catch (const std::out_of_range&) {
            throw std::runtime_error("Output index is out of range");
        }
    }();

    // print summary
    std::cout << "Audio file:      " << audiofile << '\n';
    std::cout << "- sampling rate: " << sampleRate << '\n';
    std::cout << "- channels:      " << channels << '\n';
    std::cout << "Plugin:          " << pluginKey << '\n';
    std::cout << "- output:        " << output.name << " (" << output.description << ")\n";
    std::cout << "- block size:    " << blockSize << '\n';
    std::cout << "- step size:     " << stepSize << '\n';
    std::cout << '\n';

    if (channels > 1) {
        std::cout << Escape::Yellow
            << "More than one channel provided. Only first channel will be processed!"
            << Escape::Reset << "\n\n";
    }

    // setup FFT if required
    std::optional<FFT> fft;
    if (plugin->getInputDomain() == Plugin::InputDomain::Frequency) {
        fft = FFT();
        fft->initialise(blockSize);
    }

    // initialise buffer
    const sf_count_t   readSize = blockSize * channels;
    std::vector<float> bufferInterleavedChannels(readSize);
    std::vector<float> bufferChannel(blockSize);
    std::vector<float> window(hanning(blockSize));

    // process audio block-wise, print timestamps and features
    const uint64_t nsecIncrement = (1'000'000'000 * blockSize) / sampleRate;
    uint64_t       nsec = 0;

    std::cout << "Time [s]\t" << output.name << " [" << output.unit << "]\n";

    while (file.read(bufferInterleavedChannels.data(), readSize) == readSize) {
        // deinterleave channels buffer
        const size_t channel = 0;
        for (size_t i = 0; i < blockSize; ++i) {
            bufferChannel[i] = bufferInterleavedChannels[i * channels + channel];
        }

        const auto getInputBuffer = [&]() -> Plugin::InputBuffer {
            if (plugin->getInputDomain() == Plugin::InputDomain::Frequency) {
                // multiply buffer with window (inplace)
                for (size_t i = 0; i < blockSize; ++i) {
                    bufferChannel[i] *= window[i];
                }
                return fft->compute(bufferChannel);
            } else {
                return bufferChannel;
            }
        };

        auto featureSet = plugin->process(getInputBuffer(), nsec);

        std::cout << std::fixed << nsec / 1e9 << '\t';
        for (auto&& feature : featureSet[outputIndex]) {
            std::cout << feature << '\t';
        }
        std::cout << '\n';

        nsec += nsecIncrement;
    }

    if (file.error()) {
        throw std::runtime_error(concat("Error while reading file: ", file.strError()));
    }

    std::cout << std::flush;
}

void usage(std::string_view program) {
    std::cout <<
        "usage: " << program << " [options] [plugin] [audiofile]\n"
        "\n"
        "positional arguments:\n"
        "  plugin          plugin identifier in the form library:plugin (as returned by --list-ids)\n"
        "  audiofile       path to audio file to extract features from\n"
        "\n"
        "options:\n"
        "  --output        output index (default: 0)\n"
        "  --blocksize     block size (default: preferred block size of plugin or 1024)\n"
        "\n"
        "  --list, -l      list plugin informations in human readable format\n"
        "  --list-paths    list plugin search paths\n"
        "  --list-ids      list plugins in the form library:plugin\n"
        "  --list-outputs  list plugins and outputs in the form library:plugin:output\n"
        "\n"
        "  --help, -h      show help\n"
        << std::flush;
}

int main(int argc, char* argv[]) {
    CliParser parser(argc, argv);

    if (parser.nargs() < 2 || parser.hasFlag("-h") || parser.hasFlag("--help")) {
        usage(parser.program());
        return 2;
    }

    if (parser.hasFlag("-l") || parser.hasFlag("--list")) {
        listPlugins();
        return 0;
    }

    if (parser.hasFlag("--list-paths")) {
        listPaths();
        return 0;
    }

    if (parser.hasFlag("--list-ids")) {
        listPluginIds();
        return 0;
    }

    if (parser.hasFlag("--list-outputs")) {
        listPluginOutputs();
        return 0;
    }

    if (parser.nargs() >= 3) {
        const auto plugin      = parser.args()[parser.nargs() - 2];
        const auto audiofile   = parser.args()[parser.nargs() - 1];
        const auto outputIndex = parser.getValueAs<uint32_t>("--output");
        const auto blockSize   = parser.getValueAs<uint32_t>("--blocksize");

        try {
            process(plugin, audiofile, outputIndex, blockSize);
        } catch (const std::exception& e) {
            std::cerr << Escape::Red << "[ERROR] " << e.what() << Escape::Reset << std::endl;
            return 1;
        }
        return 0;
    }

    usage(parser.program());
    return 2;
}
