# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

from datetime import date

import pkg_resources

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = "rtvamp"
copyright = f"{date.today().year}, Lukas Berbuer"
author = "Lukas Berbuer"
release = pkg_resources.get_distribution("rtvamp").version

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    "sphinx.ext.autodoc",
    "sphinx.ext.intersphinx",
    "sphinx.ext.autosummary",
    "sphinx.ext.napoleon",
    "sphinx_gallery.gen_gallery",
    "myst_parser",
]

source_suffix = [".rst", ".md"]

autosummary_generate = True
autodoc_member_order = "bysource"

templates_path = ["templates"]
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]

intersphinx_mapping = {
    "python": ("https://docs.python.org/3/", None),
    "numpy": ("https://numpy.org/doc/stable/", None),
}

from sphinx_gallery.sorting import FileNameSortKey
sphinx_gallery_conf = {
     "examples_dirs": "../examples",  # path to your example scripts
     "gallery_dirs": "_examples",  # path to where to save gallery generated output
     "filename_pattern": "",
     "ignore_pattern": r"[\\,/]_",  # files starting with an underscore
     "within_subsection_order": FileNameSortKey,
     "download_all_examples": False,
}

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = "furo"
# html_static_path = ["_static"]
