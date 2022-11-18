import os

import numpy as np
import pytest

import rtvamp

from _helper import fixture_vamp_path, get_test_library_path


def test_vamp_path_set(fixture_vamp_path):
    assert os.environ["VAMP_PATH"] is not None, "VAMP_PATH must be set to discover example plugins"


def test_get_vamp_paths():
    paths = rtvamp.get_vamp_paths()
    assert len(paths) >= 1


def test_list_libraries(fixture_vamp_path):
    libs = rtvamp.list_libraries()
    stems = [l.stem for l in libs]
    assert "example-plugin" in stems


def test_list_plugins():
    plugins = rtvamp.list_plugins()
    assert len(plugins) > 0
    assert "example-plugin:rms" in plugins


def test_load_library():
    path = get_test_library_path("example-plugin")
    library = rtvamp.load_library(path)
    assert library


def test_library():
    path = get_test_library_path("example-plugin")
    library = rtvamp.PluginLibrary(path)  # load by constructor
    assert library

    assert library.get_library_path() == path
    assert library.get_library_name() == "example-plugin"
    assert library.get_plugin_count() >= 2

    plugins = library.list_plugins()
    assert "example-plugin:rms" in plugins
    assert "example-plugin:spectralrolloff" in plugins


def test_plugin(fixture_vamp_path):
    plugin = rtvamp.load_plugin("example-plugin:rms", 48000)
    assert plugin

    assert plugin.get_vamp_api_version() == 2
    assert plugin.get_identifier() == "rms"
    assert plugin.get_name() == "RMS"
    assert plugin.get_description() == "Root mean square"
    assert plugin.get_maker() == "LB"
    assert plugin.get_copyright() == "MIT"
    assert plugin.get_plugin_version() == 1
    assert plugin.get_input_domain() == "time"
    assert plugin.get_input_samplerate() == 48000
    assert plugin.get_parameter_descriptors() == []
    assert plugin.get_programs() == []
    assert plugin.get_current_program() == None
    assert plugin.get_preferred_stepsize() == 0
    assert plugin.get_preferred_blocksize() == 0
    assert plugin.get_output_count() == 1
    outputs = plugin.get_output_descriptors()
    assert len(outputs) == 1
    assert outputs[0] == dict(
        identifier="rms",
        name="RMS",
        description="Root mean square of signal",
        unit="V",
        bin_count=1,
        bin_names=[],
        has_known_extents=False,
        min_value=0,
        max_value=0,
        quantize_step=None,
    )

    # process before init should raise exception (with RTVAMP_VALIDATE compile flag)
    with pytest.raises(Exception):
        plugin.process(np.zeros(16), nsec=0)

    plugin.initialise(stepsize=16, blocksize=16)

    # process with wrong buffer size should raise exception (with RTVAMP_VALIDATE compile flag)
    with pytest.raises(Exception):
        plugin.process(np.zeros(4), nsec=0)

    # process with wrong input domain should raise exception
    with pytest.raises(ValueError):
        input_freqdomain = np.zeros(18).astype(np.complex64)
        plugin.process(input_freqdomain, nsec=0)

    input_timedomain = np.zeros(16).astype(np.float32)
    result = plugin.process(input_timedomain, nsec=0)
    assert result == [[0.0]]
