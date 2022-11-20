from pathlib import Path
from setuptools import find_packages
from subprocess import run
from tempfile import TemporaryDirectory

from skbuild import setup

HERE = Path(__file__).parent

LONG_DESCRIPTION = (HERE / "python" / "README.md").read_text(encoding="utf-8")

INSTALL_REQUIRES = [
    "numpy",
    "dataclasses>=0.6; python_version<'3.7'",
]

EXTRAS_REQUIRE = {
    "docs": [
        "sphinx>=5",
        "sphinx-gallery",
        "myst-parser",
        "furo",
        # for examples
        "librosa",
        "matplotlib",
    ],
    "tests": [
        "pytest>=6",  # pyproject.toml support
    ],
    "tools": [
        "black",
        "isort",
        "mypy>=0.9",  # pyproject.toml support
        "pylint>=2.5",  # pyproject.toml support
        "tox>=3.4",  # pyproject.toml support
    ],
}

EXTRAS_REQUIRE["dev"] = EXTRAS_REQUIRE["docs"] + EXTRAS_REQUIRE["tests"] + EXTRAS_REQUIRE["tools"]


def get_variable_from_cmake(variable: str) -> str:
    """Read CMake variables from temporary generated CMakeCache.txt."""
    try:
        with TemporaryDirectory() as tempdir:
            run(["cmake", "-S", str(HERE), "-B", tempdir], check=True)
            with open(Path(tempdir) / "CMakeCache.txt", encoding="utf-8") as f:
                for line in f:
                    if line.startswith(f"{variable}:"):
                        *_, version = line.strip().partition("=")
                        return version
            raise RuntimeError(f"{variable} not found in CMakeCache.txt")
    except Exception as e:
        raise RuntimeError(f"Could not fetch variable from CMake: {e}") from e


setup(
    name="rtvamp",
    version=get_variable_from_cmake("CMAKE_PROJECT_VERSION"),
    description="Vamp plugin host for real-time audio feature analysis",
    long_description=LONG_DESCRIPTION,
    long_description_content_type="text/markdown",
    url="https://github.com/lukasberbuer/rt-vamp-plugin-sdk",
    author="Lukas Berbuer",
    author_email="lukas.berbuer@gmail.com",
    license="MIT",
    classifiers=[
        "Development Status :: 4 - Beta",
        "Intended Audience :: Developers",
        "Intended Audience :: Science/Research",
        "Topic :: Scientific/Engineering",
        "Topic :: Multimedia :: Sound/Audio",
        "Topic :: Multimedia :: Sound/Audio :: Analysis",
        "License :: OSI Approved :: MIT License",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.6",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Operating System :: OS Independent",
    ],
    keywords=[
        "Vamp",
        "plugin",
        "audio",
        "sound",
        "music",
        "analysis",
        "feature extraction",
        "real-time",
    ],
    packages=find_packages(where="python/src"),
    package_dir={"": "python/src"},
    python_requires=">=3.6",
    install_requires=INSTALL_REQUIRES,
    extras_require=EXTRAS_REQUIRE,
    cmake_args=[
        "-DRTVAMP_BUILD_EXAMPLES=ON",  # to test with example plugins
        "-DRTVAMP_BUILD_PYTHON_BINDINGS=ON",
        "-DRTVAMP_VALIDATE=ON",
    ],
    cmake_install_dir="python/src/rtvamp",
    include_package_data=False,
)
