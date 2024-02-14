# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.3.1] - 2024-02-14

### Fixed

- Missing Python 3.12 wheel

## [0.3.0] - 2024-02-14

### Added

- Python 3.12 support

### Fixed

- Missing `<mutex>` import for `std::unique_lock` in `PluginAdapter.hpp`

## [0.2.0] - 2022-11-20

### Added

- Python bindings for hostsdk & documentation
- CI for Python bindings (test, wheels, publish to PyPI)
- `RTVAMP_VALIDATE` compile option for hostsdk to validate input data and method call order
- `pluginsdk::Plugin::getLibraryPath` method

### Changed

- Replace `hostsdk::PluginLoader` class with functions:
  - `hostsdk::getVampPaths`
  - `hostsdk::listLibraries`
  - `hostsdk::listPlugins`
  - `hostsdk::loadLibrary`
  - `hostsdk::loadPlugin`
- Set default `binCount` of `pluginsdk::Plugin::OutputDescriptor` to 1 (instead of 0)
- Use optional as return type for `hostsdk::Plugin::getCurrentProgram`
- Use `string_view` for `pluginsdk::Plugin::ProgramList` (instead of const char\* const)

### Fixed

- `hostsdk::Plugin::getPluginPaths` if `$HOME` env variable not set
- `hostsdk::Plugin::getCurrentProgram` for empty program lists
- `RTVAMP_ENTRY_POINT` macro and symbol export for MSVC Win32 builds
- Zero-crossing plugin
- Handle or prevent copy/move of pluginsdk wrapper classes (pointer invalidation)

## [0.1.0] - 2022-01-17

First release

[unreleased]: https://github.com/lukasberbuer/rt-vamp-plugin-sdk/compare/v0.3.1...HEAD
[0.3.1]: https://github.com/lukasberbuer/rt-vamp-plugin-sdk/compare/v0.3.0...v0.3.1
[0.3.0]: https://github.com/lukasberbuer/rt-vamp-plugin-sdk/compare/v0.2.0...v0.3.0
[0.2.0]: https://github.com/lukasberbuer/rt-vamp-plugin-sdk/compare/v0.1.0...v0.2.0
[0.1.0]: https://github.com/lukasberbuer/rt-vamp-plugin-sdk/releases/tag/v0.1.0
