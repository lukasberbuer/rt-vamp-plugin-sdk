from dataclasses import dataclass
from os import PathLike
from typing import Any, Dict, List, Optional, Tuple, Union

import numpy as np
from numpy.lib.stride_tricks import as_strided

from rtvamp._bindings import (
    Plugin,
    PluginLibrary,
    get_vamp_paths,
    list_libraries,
    list_plugins,
    load_library,
    load_plugin,
)


@dataclass
class PluginMetadata:
    """Aggregated plugin metadata."""

    identifier: str
    name: str
    description: str
    maker: str
    copyright: str
    plugin_version: int
    input_domain: str
    parameter_descriptors: List[Dict[str, Any]]
    output_descriptors: List[Dict[str, Any]]


def get_plugin_metadata(key: str, samplerate: float = 48000) -> PluginMetadata:
    """
    Get all the plugin metadata and descriptors.

    Note:
        The output descriptors may depend on parameter values and the initialised step- and block sizes.

    Args:
        key: Plugin key/identifer as returned by e.g. :func:`list_plugins`

    Returns:
        Aggregated plugin metadata.
    """
    plugin = load_plugin(key, samplerate)
    return PluginMetadata(
        identifier=plugin.get_identifier(),
        name=plugin.get_name(),
        description=plugin.get_description(),
        maker=plugin.get_maker(),
        copyright=plugin.get_copyright(),
        plugin_version=plugin.get_plugin_version(),
        input_domain=plugin.get_input_domain(),
        parameter_descriptors=plugin.get_parameter_descriptors(),
        output_descriptors=plugin.get_output_descriptors(),
    )


def _get_plugin_key(plugin):
    library_name = plugin.get_library_path().stem
    plugin_id = plugin.get_identifier()
    return f"{library_name}:{plugin_id}"


def _get_plugin_output_identifier(plugin):
    plugin_key = _get_plugin_key(plugin)
    for output in plugin.get_output_descriptors():
        output_id = output["identifier"]
        yield f"{plugin_key}:{output_id}"


Feature = List[float]
FeatureList = List[Feature]


def _frame_count(nsamples: int, blocksize: int, stepsize: int):
    return (nsamples - blocksize) // stepsize + 1  # nsamples = blocksize + (nframes - 1) * stepsize


def _frame(x: np.ndarray, blocksize: int, stepsize: int):
    """Slice a array into (overlapping) frames."""
    x = np.array(x, copy=False)
    blocksize = int(blocksize)
    stepsize =int(stepsize)

    if x.ndim != 1:
        raise ValueError(f"Invalid array dimension: {x.ndim}")
    if blocksize < 1:
        raise ValueError(f"Invalid blocksize: {blocksize}")
    if stepsize < 1:
        raise ValueError(f"Invalid stepsize: {stepsize}")
    if blocksize > x.shape[0]:
        raise ValueError(f"Input too short ({x.shape[0]}) for blocksize={blocksize}")

    nsamples = x.shape[0]
    nframes = _frame_count(nsamples, blocksize, stepsize)
    out_shape = [nframes, blocksize]

    stride_dtype = x.strides[0]  # = bytes of dtype
    out_strides = [stepsize * stride_dtype, stride_dtype]

    return as_strided(x, shape=out_shape, strides=out_strides, writeable=False)


class FeatureComputation:
    """Compute features with (multiple) plugins block-wise or from a stream (advanced usage)."""

    def __init__(self, samplerate: float):
        """
        Initialize `FeatureComputation` class.

        Args:
            samplerate: Input sample rate
        """
        self._samplerate = samplerate
        self._plugins = []
        self._outputs = []
        self._blocksize = 0
        self._stepsize = 0
        self._window = np.empty(0, dtype=np.float32)

    @property
    def plugins(self) -> List[Plugin]:
        """List of added plugins."""
        return self._plugins

    @property
    def outputs(self) -> List[str]:
        """List of all plugin outputs."""
        return self._outputs

    def add_plugin(
        self,
        key: str,
        parameter: Optional[Dict[str, float]] = None,
        paths: Optional[List[PathLike]] = None,
    ):
        """
        Add plugin for processing.

        Args:
            key: Plugin key/identifer as returned by e.g. :func:`list_plugins`
            parameter: Dict with parameter identifiers and values.
                Use :func:`get_plugin_metadata` or :func:`Plugin.get_parameter_descriptors` to list
                available parameters and their constraints.
            paths: Custom paths, either search paths or plugin library paths
        """
        plugin = load_plugin(key=key, samplerate=self._samplerate, paths=paths)
        for key, value in (parameter or {}).items():
            success = plugin.set_parameter(key, value)
            if not success:
                raise ValueError(f"Invalid parameter {key}")
        self._plugins.append(plugin)
        self._outputs.append(*_get_plugin_output_identifier(plugin))

    def initialise(self, blocksize: int, stepsize: Optional[int] = None):
        """
        Initialise all added plugins.

        Args:
            blocksize: Block size in samples
            stepsize: Step size in samples (< `blocksize`, default = `blocksize`)
        """
        self._blocksize = blocksize
        self._stepsize = stepsize or blocksize
        self._window = np.hanning(blocksize).astype(np.float32, copy=False)
        for plugin in self._plugins:
            success = plugin.initialise(
                stepsize=self._stepsize,
                blocksize=self._blocksize,
            )
            if not success:
                raise RuntimeError(f"Failed to initialise plugin {plugin.get_identifier()}")

    def reset(self):
        """Reset all added plugins."""
        for plugin in self._plugins:
            plugin.reset()

    def get_output_descriptors(self):
        """Get output descriptors."""
        return [output for plugin in self._plugins for output in plugin.get_output_descriptors()]

    def _is_frequency_domain_required(self):
        return any((plugin.get_input_domain() == "frequency" for plugin in self._plugins))

    def process_block(self, timedata_block: np.ndarray, timestamp: float) -> FeatureList:
        """
        Process a single block/frame of data.

        Args:
            timedata_block: Single block/frame of time series data
                (length equal to initialised `blocksize`)
            timestamp: Timestamp of block in seconds
        
        Returns:
            List of computed features (same length as :attr:`~outputs`).

            The feature itself is list of floats (check `bin_count` with :func:`get_output_descriptors`).
        """
        timedata_block = timedata_block.astype(np.float32, copy=False)
        fft_block = (
            np.fft.rfft(timedata_block * self._window).astype(np.complex64, copy=False)
            if self._is_frequency_domain_required()
            else None
        )

        results = []
        for plugin in self._plugins:
            block = fft_block if plugin.get_input_domain() == "frequency" else timedata_block
            result = plugin.process(block, int(timestamp * 1e9))
            results.extend(result)
        return results

    def process_signal(
        self,
        timedata: np.ndarray,
        timestamp_start: float = 0,
    ) -> Tuple[np.ndarray, List[np.ndarray]]:
        """
        Process data of arbitrary length.

        Args:
            timedata: Time series data of arbitrary length.
                Signal will be cropped to blocks/frames accoring to initialised `stepsize` and `blocksize`.
            timestamp_start: Timestamp of signal start in seconds

        Returns:
            - Array of timestamps in seconds
            - List of arrays of computed features (same length as timestamps).
              Check :attr:`~outputs` to map the arrays to the plugin outputs.
        """
        nframes = _frame_count(timedata.shape[0], self._blocksize, self._stepsize)
        outputs = [
            np.empty(shape=(d["bin_count"], nframes), dtype=np.float32)
            for d in self.get_output_descriptors()
        ]

        timestamp_inc = self._stepsize / self._samplerate
        timestamp = timestamp_start
        for i, timedata_block in enumerate(
            _frame(timedata, blocksize=self._blocksize, stepsize=self._stepsize)
        ):
            results = self.process_block(timedata_block, timestamp=timestamp)
            assert len(outputs) == len(results)
            for output, result in zip(outputs, results):
                output[:, i] = result
            timestamp += timestamp_inc

        timestamps = np.arange(0, nframes) * timestamp_inc + timestamp_start
        return timestamps, outputs


def compute_features(
    timedata: np.ndarray,
    samplerate: float,
    plugin: str,
    *,
    blocksize: Optional[int] = None,
    stepsize: Optional[int] = None,
    parameter: Optional[Dict[str, float]] = None,
) -> Tuple[np.ndarray, List[np.ndarray]]:
    """
    Compute features with plugin.

    Args:
        timedata: Time series data of arbitrary length
        samplerate: Sampling rate in Hz
        plugin: Plugin key/identifer as returned by e.g. :func:`list_plugins`
        blocksize: Block size in samples (default: preferred block size of plugin, otherwise 1024)
        stepsize: Step size in samples (default: preferred step size of plugin, otherwise = `blocksize`)
        parameter: Dict with parameter identifiers and values.
            Use :func:`get_plugin_metadata` or :func:`Plugin.get_parameter_descriptors` to list
            available parameters and their constraints.

    Returns:
        - Array of timestamps in seconds
        - Array of computed features
    """
    proc = FeatureComputation(samplerate=samplerate)
    proc.add_plugin(plugin, parameter=parameter)

    assert len(proc.plugins) == 1
    plugin = proc.plugins[0]
    blocksize = (
        blocksize or
        plugin.get_preferred_blocksize() or
        1024
    )
    stepsize = (
        stepsize or
        plugin.get_preferred_stepsize() or
        blocksize
    )

    proc.initialise(stepsize=stepsize, blocksize=blocksize)
    timestamps, outputs = proc.process_signal(timedata)
    assert len(outputs) == 1
    return timestamps, outputs[0]
