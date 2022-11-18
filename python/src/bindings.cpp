#include <cassert>
#include <complex>
#include <filesystem>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/complex.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/numpy.h>

#include "rtvamp/hostsdk.hpp"

namespace py = pybind11;
using namespace pybind11::literals;

using Plugin        = rtvamp::hostsdk::Plugin;
using PluginKey     = rtvamp::hostsdk::PluginKey;
using PluginLibrary = rtvamp::hostsdk::PluginLibrary;

using PyTimeDomainBuffer      = py::array_t<float, py::array::c_style | py::array::forcecast>;
using PyFrequencyDomainBuffer = py::array_t<std::complex<float>, py::array::c_style | py::array::forcecast>;
using PyInputBuffer           = std::variant<PyTimeDomainBuffer, PyFrequencyDomainBuffer>;

/**
 * Trampoline for Plugin class.
 * https://pybind11.readthedocs.io/en/stable/advanced/classes.html
 */
class PyPlugin : public Plugin {
public:
    using Plugin::Plugin;

    std::filesystem::path getLibraryPath() const noexcept override {
        PYBIND11_OVERRIDE_PURE(std::filesystem::path, Plugin, getLibraryPath);
    }
    uint32_t getVampApiVersion() const noexcept override {
        PYBIND11_OVERRIDE_PURE(uint32_t, Plugin, getVampApiVersion);
    }
    std::string_view getIdentifier() const noexcept override {
        PYBIND11_OVERRIDE_PURE(std::string_view, Plugin, getIdentifier);
    }
    std::string_view getName() const noexcept override {
        PYBIND11_OVERRIDE_PURE(std::string_view, Plugin, getName);
    }
    std::string_view getDescription() const noexcept override {
        PYBIND11_OVERRIDE_PURE(std::string_view, Plugin, getDescription);
    }
    std::string_view getMaker() const noexcept override {
        PYBIND11_OVERRIDE_PURE(std::string_view, Plugin, getMaker);
    }
    std::string_view getCopyright() const noexcept override {
        PYBIND11_OVERRIDE_PURE(std::string_view, Plugin, getCopyright);
    }
    int getPluginVersion() const noexcept override {
        PYBIND11_OVERRIDE_PURE(int, Plugin, getPluginVersion);
    }
    InputDomain getInputDomain() const noexcept override {
        PYBIND11_OVERRIDE_PURE(InputDomain, Plugin, getInputDomain);
    }
    ParameterList getParameterDescriptors() const noexcept override {
        PYBIND11_OVERRIDE_PURE(ParameterList, Plugin, getParameterDescriptors);
    }
    std::optional<float> getParameter(std::string_view id) const override {
        PYBIND11_OVERRIDE_PURE(std::optional<float>, Plugin, getParameter, id);
    }
    bool setParameter(std::string_view id, float value) override {
        PYBIND11_OVERRIDE_PURE(bool, Plugin, setParameter, id, value);
    }
    ProgramList getPrograms() const noexcept override {
        PYBIND11_OVERRIDE_PURE(ProgramList, Plugin, getPrograms);
    }
    CurrentProgram getCurrentProgram() const override {
        PYBIND11_OVERRIDE_PURE(std::string_view, Plugin, getCurrentProgram);
    }
    bool selectProgram(std::string_view name) override {
        PYBIND11_OVERRIDE_PURE(bool, Plugin, selectProgram, name);
    }
    uint32_t getPreferredStepSize() const override {
        PYBIND11_OVERRIDE_PURE(uint32_t, Plugin, getPreferredStepSize);
    }
    uint32_t getPreferredBlockSize() const override {
        PYBIND11_OVERRIDE_PURE(uint32_t, Plugin, getPreferredBlockSize);
    }
    uint32_t getOutputCount() const override {
        PYBIND11_OVERRIDE_PURE(uint32_t, Plugin, getOutputCount);
    }
    OutputList getOutputDescriptors() const override {
        PYBIND11_OVERRIDE_PURE(OutputList, Plugin, getOutputDescriptors);
    }
    bool initialise(uint32_t stepSize, uint32_t blockSize) override {
        PYBIND11_OVERRIDE_PURE(bool, Plugin, initialise, stepSize, blockSize);
    }
    void reset() override {
        PYBIND11_OVERRIDE_PURE(void, Plugin, reset);
    }
    FeatureSet process(InputBuffer buffer, uint64_t nsec) override {
        PYBIND11_OVERRIDE_PURE(FeatureSet, Plugin, process, buffer, nsec);
    }
};

static auto convertPluginKeys(const std::vector<PluginKey>& pluginKeys) {
    std::vector<std::string> result;
    for (auto&& key : pluginKeys) {
        result.emplace_back(key.get());
    }
    return result;
}

template <typename TSpan, typename TVector = TSpan>
static auto convertSpanToVector(std::span<const TSpan> s) {
    return std::vector<TVector>(s.begin(), s.end());
}

template <typename T, int ExtraFlags>
static std::span<const T> convertNumpyArrayToSpan(const py::array_t<T, ExtraFlags>& numpyArray) {
    // https://pybind11.readthedocs.io/en/stable/advanced/pycpp/numpy.html
    const auto bufferInfo = numpyArray.request();
    if (bufferInfo.ndim != 1) {
        throw std::invalid_argument("Numpy array dimension must be 1");
    }
    return {
        static_cast<const T*>(bufferInfo.ptr),
        static_cast<size_t>(bufferInfo.shape[0])
    };
}

PYBIND11_MODULE(_bindings, m) {
    m.def(
        "get_vamp_paths",
        &rtvamp::hostsdk::getVampPaths,
        "Get default search paths for Vamp plugin libraries."
    );

    m.def(
        "list_libraries",
        [](std::optional<std::vector<std::filesystem::path>> paths) {
            if (paths) {
                return rtvamp::hostsdk::listLibraries(paths.value());
            } else {
                return rtvamp::hostsdk::listLibraries();
            }
        },
        R"pbdoc(
            List all plugin libraries.

            Args:
                paths: Custom search paths (default: :func:`get_vamp_paths`)

            Returns:
                Paths of found libraries
        )pbdoc",
        py::arg("paths") = std::nullopt
    );

    m.def(
        "list_plugins",
        [](std::optional<std::vector<std::filesystem::path>> paths) {
            if (paths) {
                return convertPluginKeys(rtvamp::hostsdk::listPlugins(paths.value()));
            } else {
                return convertPluginKeys(rtvamp::hostsdk::listPlugins());
            }
        },
        R"pbdoc(
            List all plugins.

            Args:
                paths: Custom paths, either search paths or plugin library paths

            Returns:
                List of plugin keys/identifiers
        )pbdoc",
        py::arg("paths") = std::nullopt
    );

    m.def(
        "load_library",
        &rtvamp::hostsdk::loadLibrary,
        R"pbdoc(
            Load plugin library by file path.

            Args:
                path: Path to plugin library

            Returns:
                :class:`PluginLibrary` instance
        )pbdoc",
        py::arg("path"),
        py::return_value_policy::take_ownership
    );

    m.def(
        "load_plugin",
        [](std::string_view key, float samplerate, std::optional<std::vector<std::filesystem::path>> paths) {
            if (paths) {
                return rtvamp::hostsdk::loadPlugin(key, samplerate, paths.value());
            } else {
                return rtvamp::hostsdk::loadPlugin(key, samplerate);
            }
        },
        R"pbdoc(
            Load plugin.

            Args:
                key: Plugin key/identifer as returned by e.g. :func:`list_plugins`
                samplerate: Input sample rate
                paths: Custom paths, either search paths or plugin library paths

            Returns:
                :class:`Plugin` instance
        )pbdoc",
        py::arg("key"),
        py::arg("samplerate"),
        py::arg("paths") = std::nullopt,
        py::return_value_policy::take_ownership
    );

    py::class_<PluginLibrary>(
        m,
        "PluginLibrary",
        "Plugin library interface to inspect and load plugins."
    )
        .def(py::init<const std::filesystem::path&>(), py::arg("path"))
        .def("get_library_path", &PluginLibrary::getLibraryPath)
        .def("get_library_name", &PluginLibrary::getLibraryName)
        .def("get_plugin_count", &PluginLibrary::getPluginCount)
        .def("list_plugins", [](const PluginLibrary& self) {
            return convertPluginKeys(self.listPlugins());
        })
        .def(
            "load_plugin", [](const PluginLibrary& self, std::string_view key, float inputSampleRate) {
                return self.loadPlugin(key, inputSampleRate);
            },
            py::arg("key"),
            py::arg("samplerate"),
            py::return_value_policy::take_ownership
        );

    py::class_<Plugin, PyPlugin /* trampoline */>(
        m,
        "Plugin",
        R"pbdoc(
            Plugin base class.

            Must be instantiated by the :func:`load_plugin` function or via the :class:`PluginLibrary` class.
        )pbdoc"
    )
        .def(py::init<float>(), py::arg("samplerate"))
        .def("get_library_path", &Plugin::getLibraryPath)
        .def("get_vamp_api_version", &Plugin::getVampApiVersion)
        .def("get_identifier", &Plugin::getIdentifier)
        .def("get_name", &Plugin::getName)
        .def("get_description", &Plugin::getDescription)
        .def("get_maker", &Plugin::getMaker)
        .def("get_copyright", &Plugin::getCopyright)
        .def("get_plugin_version", &Plugin::getPluginVersion)
        .def("get_input_domain", [](const Plugin& self) {
            return (self.getInputDomain() == Plugin::InputDomain::Frequency) ? "frequency" : "time";
        })
        .def("get_input_samplerate", &Plugin::getInputSampleRate)
        .def("get_parameter_descriptors", [](const Plugin& self) {
            std::vector<py::dict> result;
            for (auto&& d : self.getParameterDescriptors()) {
                result.emplace_back(
                    "identifier"_a    = d.identifier,
                    "name"_a          = d.name,
                    "description"_a   = d.description,
                    "unit"_a          = d.unit,
                    "default_value"_a = d.defaultValue,
                    "min_value"_a     = d.minValue,
                    "max_value"_a     = d.maxValue,
                    "quantize_step"_a = d.quantizeStep,
                    "value_names"_a   = d.valueNames
                );
            }
            return result;
        })
        .def("get_parameter", &Plugin::getParameter, py::arg("id"))
        .def("set_parameter", &Plugin::setParameter, py::arg("id"), py::arg("value"))
        .def("get_programs", [](const Plugin& self) {
            return convertSpanToVector(self.getPrograms());
        })
        .def("get_current_program", &Plugin::getCurrentProgram)
        .def("select_program", &Plugin::selectProgram, py::arg("name"))
        .def("get_preferred_stepsize", &Plugin::getPreferredStepSize)
        .def("get_preferred_blocksize", &Plugin::getPreferredBlockSize)
        .def("get_output_count", &Plugin::getOutputCount)
        .def("get_output_descriptors", [](const Plugin& self) {
            std::vector<py::dict> result;
            for (auto&& d : self.getOutputDescriptors()) {
                result.emplace_back(
                    "identifier"_a        = d.identifier,
                    "name"_a              = d.name,
                    "description"_a       = d.description,
                    "unit"_a              = d.unit,
                    "bin_count"_a         = d.binCount,
                    "bin_names"_a         = d.binNames,
                    "has_known_extents"_a = d.hasKnownExtents,
                    "min_value"_a         = d.minValue,
                    "max_value"_a         = d.maxValue,
                    "quantize_step"_a     = d.quantizeStep
                );
            }
            return result;
        })
        .def("initialise", &Plugin::initialise, py::arg("stepsize"), py::arg("blocksize"))
        .def("reset", &Plugin::reset)
        .def(
            "process",
            [](Plugin& self, PyInputBuffer input, uint64_t nsec) {
                return std::visit(
                    [&](auto&& numpyArray) {
                        auto buffer = convertNumpyArrayToSpan(numpyArray);
                        return convertSpanToVector(self.process(buffer, nsec));
                    },
                    input
                );
            },
            py::arg("array"),
            py::arg("nsec")
        );
}
