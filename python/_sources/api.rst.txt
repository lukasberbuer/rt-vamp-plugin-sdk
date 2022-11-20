API
===

High-level
----------

.. autosummary::
    :caption: API high-level
    :toctree: _generated
    :nosignatures:
    :recursive:

    rtvamp.get_vamp_paths
    rtvamp.list_libraries
    rtvamp.list_plugins
    rtvamp.get_plugin_metadata
    rtvamp.PluginMetadata
    rtvamp.compute_features
    rtvamp.FeatureComputation


Low-level
---------

Direct bindings to the C++ hostsdk (`GitHub <https://github.com/lukasberbuer/rt-vamp-plugin-sdk>`_, `API <https://lukasberbuer.github.io/rt-vamp-plugin-sdk/>`_).

.. autosummary::
    :caption: API low-level
    :toctree: _generated
    :nosignatures:
    :recursive:

    rtvamp.load_library
    rtvamp.load_plugin
    rtvamp.PluginLibrary
    rtvamp.Plugin
