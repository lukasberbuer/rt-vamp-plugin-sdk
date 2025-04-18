import os
import platform
from pathlib import Path

import pytest
import rtvamp


def get_library_extension():
    system = platform.system()
    if system == "Linux":
        return ".so"
    if system == "Windows":
        return ".dll"
    if system == "Darwin":
        return ".dylib"
    raise RuntimeError("Unknown platform")  # noqa: EM101


def get_test_library_folder() -> Path:
    """Example plugins for testing are located in the package's plugins directory."""
    return Path(rtvamp.__file__).parent / "plugins"


def get_test_library_path(filename: str) -> Path:
    return get_test_library_folder() / Path(filename).with_suffix(get_library_extension())


@pytest.fixture
def fixture_vamp_path():
    os.environ["VAMP_PATH"] = str(get_test_library_folder().resolve())
