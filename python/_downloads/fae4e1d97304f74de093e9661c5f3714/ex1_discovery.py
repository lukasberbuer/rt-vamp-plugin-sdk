"""
Plugin discovery and browsing
=============================
"""

import os
from pathlib import Path
from pprint import pprint

import rtvamp

# set VAMP_PATH to rtvamp package dir to find example plugins
os.environ["VAMP_PATH"] = os.path.dirname(rtvamp.__file__)

#%%
# Browse available libraries and plugins
# --------------------------------------
# List libraries:
rtvamp.list_libraries()

#%%
# List plugins:
rtvamp.list_plugins()

#%%
# List plugins in current working directory:
rtvamp.list_plugins(paths=[Path.cwd()])

#%%
# Get metadata and descriptors from plugin
for plugin in rtvamp.list_plugins():
    try:
        metadata = rtvamp.get_plugin_metadata(plugin)
        pprint(metadata)
    except:
        print(f"Could not get metadata from plugin {plugin}")
