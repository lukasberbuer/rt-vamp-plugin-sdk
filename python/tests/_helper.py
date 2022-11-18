import os
import platform
from pathlib import Path

import pytest
import rtvamp


def get_library_extension():
    system = platform.system()
    if system == "Linux":
        return ".so"
    elif system == "Windows":
        return ".dll"
    elif system == "Darwin":
        return ".dylib"
    raise RuntimeError("Unknown platform")


def get_test_library_folder() -> Path:
    """Example plugins for testing are located in the package root directory."""
    return Path(rtvamp.__file__).parent.resolve()


def get_test_library_path(filename: str) -> Path:
    return get_test_library_folder() / Path(filename).with_suffix(get_library_extension())


@pytest.fixture
def fixture_vamp_path():
    os.environ["VAMP_PATH"] = str(get_test_library_folder())
