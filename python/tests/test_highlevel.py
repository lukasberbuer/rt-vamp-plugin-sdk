import numpy as np
import pytest
from numpy.testing import assert_allclose

from rtvamp import FeatureComputation, _frame, compute_features

from _helper import fixture_vamp_path


def test_feature_computation(fixture_vamp_path):
    proc = FeatureComputation(samplerate=1000)

    with pytest.raises(ValueError):
        proc.add_plugin("invalidplugin:idontknow")

    with pytest.raises(ValueError):
        proc.add_plugin("example-plugin:rms", {"invalidparam": 0.1})

    proc.add_plugin("example-plugin:rms")
    proc.add_plugin("example-plugin:spectralrolloff", {"rolloff": 0.5})

    assert len(proc.outputs) == 2
    assert len(proc.get_output_descriptors()) == 2

    proc.initialise(10, 10)
    timestamps, outputs = proc.process_signal(np.ones(100), 10)
    assert timestamps.shape == (10,)
    assert len(outputs) == 2
    assert outputs[0].shape == (1, 10)
    assert outputs[1].shape == (1, 10)


def rms(x: np.ndarray):
    return np.sqrt(np.mean(x ** 2))


def rms_blockwise(x: np.ndarray, blocksize: int, stepsize):
    n = len(x)
    timestamps = np.arange(0, n - blocksize + 1, stepsize)
    output_rms = np.asarray([rms(xi) for xi in _frame(x, blocksize, stepsize)], dtype=np.float32)
    assert timestamps.shape == output_rms.shape
    return timestamps, output_rms


@pytest.mark.parametrize(
    ("blocksize, stepsize"),
    (
        (10, None),
        (10, 10),
        (10, 5),
        (10, 2),
        (1, 1),
    )
)
def test_feature_computation_rms(fixture_vamp_path, blocksize, stepsize):
    proc = FeatureComputation(samplerate=1)
    proc.add_plugin("example-plugin:rms")
    proc.initialise(blocksize=blocksize, stepsize=stepsize)

    x = np.random.randn(111)
    timestamps, outputs = proc.process_signal(x)
    output_rms = outputs[0][0]  # first output, first bin#

    timestamps_expected, output_rms_expected = rms_blockwise(x, blocksize, stepsize or blocksize)

    assert_allclose(timestamps, timestamps_expected)
    assert_allclose(output_rms, output_rms_expected, rtol=1e-6)


@pytest.mark.parametrize(
    ("blocksize, stepsize"),
    (
        (10, None),
        (10, 10),
        (10, 5),
        (10, 2),
        (1, 1),
    )
)
def test_compute_features_rms(fixture_vamp_path, blocksize, stepsize):
    x = np.random.randn(111)
    timestamps, output = compute_features(
        x,
        samplerate=1,
        plugin="example-plugin:rms",
        blocksize=blocksize,
        stepsize=stepsize,
    )
    output_rms = output[0]  # first bin -> use np.squeeze?

    timestamps_expected, output_rms_expected = rms_blockwise(x, blocksize, stepsize or blocksize)

    assert_allclose(timestamps, timestamps_expected)
    assert_allclose(output_rms, output_rms_expected, rtol=1e-6)
