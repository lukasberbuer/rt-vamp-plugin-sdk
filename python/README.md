# rtvamp

Python bindings of Vamp plugin host for real-time audio feature analysis.

- [API documentation Python](https://lukasberbuer.github.io/rt-vamp-plugin-sdk/python)
- [API documentation C++](https://lukasberbuer.github.io/rt-vamp-plugin-sdk)
- [Examples](https://github.com/lukasberbuer/rt-vamp-plugin-sdk/tree/master/python/examples)

## Installation

Install the latest version from PyPI:

```shell
$ pip install rtvamp
```

## Simple example

Using [librosa](https://librosa.org) to read an audio file and the [spectral roll-off plugin](https://github.com/lukasberbuer/rt-vamp-plugin-sdk/tree/master/examples/plugin) for analysis:

```python
>>> import rtvamp
>>> import librosa

>>> rtvamp.list_plugins()
['example-plugin:rms', 'example-plugin:spectralrolloff']

>>> rtvamp.get_plugin_metadata("example-plugin:spectralrolloff")
PluginMetadata(identifier='spectralrolloff', name='Spectral roll-off', description='', maker='LB', copyright='MIT', plugin_version=1, input_domain='frequency', parameter_descriptors=[{'identifier': 'rolloff', 'name': 'Roll-off factor', 'description': 'Some random parameter', 'unit': '', 'default_value': 0.8999999761581421, 'min_value': 0.0, 'max_value': 1.0, 'quantize_step': None, 'value_names': []}], output_descriptors=[{'identifier': 'frequency', 'name': 'Roll-off frequency', 'description': 'Frequency below which n% of the total energy is concentrated', 'unit': 'Hz', 'bin_count': 1, 'bin_names': [], 'has_known_extents': False, 'min_value': 0.0, 'max_value': 0.0, 'quantize_step': None}])

>>> y, sr = librosa.load(librosa.ex("trumpet"))
>>> t_rolloff, rolloff = rtvamp.compute_features(
...     y, sr,
...     plugin="example-plugin:spectralrolloff",
...     blocksize=256,
...     parameter={"rolloff": 0.5},
... )
>>> rolloff
array([[1291.992188, 1291.992188, 1722.656250, ..., 5684.765625,
        5598.632812, 6459.960938]], dtype=float32)
```
