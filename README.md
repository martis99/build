# build

cross-platform build system generator 

[![codecov](https://codecov.io/github/martis99/build/branch/main/graph/badge.svg?token=YBT0AA2GPG)](https://codecov.io/github/martis99/build)
[![Coverage Status](https://coveralls.io/repos/github/martis99/build/badge.svg)](https://coveralls.io/github/martis99/build)

## Features

| Support | Linux <br> [![build](https://github.com/martis99/build/actions/workflows/build-linux.yml/badge.svg)](https://github.com/martis99/build/actions/workflows/build-linux.yml) | Windows <br> [![build](https://github.com/martis99/build/actions/workflows/build-windows.yml/badge.svg)](https://github.com/martis99/build/actions/workflows/build-windows.yml) |
| ----- | :-: | :-: |
MSBuild |     | [![test](https://github.com/martis99/build/actions/workflows/test-windows-msbuild.yml/badge.svg)](https://github.com/martis99/build/actions/workflows/test-windows-msbuild.yml) |
CMake   |     | [![test](https://github.com/martis99/build/actions/workflows/test-windows-cmake.yml/badge.svg)](https://github.com/martis99/build/actions/workflows/test-windows-cmake.yml) |
Make    | [![test](https://github.com/martis99/build/actions/workflows/test-linux-make.yml/badge.svg)](https://github.com/martis99/build/actions/workflows/test-linux-make.yml) |     |
QMake   |     |     |
Ninja   |     |     |
Premake |     |     |
Meson   |     |     |

## Usage

### Windows x64

Run `build-win-x64.exe` in solution directory.  

### Windows x86

Run `build-win-x86.exe` in solution directory.  

### Linux

Run `build-linux` in solution directory.  

### Arguments

To change build system add `-G` argument with one of the options:
| Option |   Build System   | Default on |
| ------ |----------------- | ---------- |
| V      | Visual Studio 17 | Windows    |
| C      | CMake            |            |
| M      | Make             |            |
| W      | VS Code          | Linux      |

Example: `./build-linux -G C`

To change solution directory add `-S` argument with solution direcory

Example: `./build-linux -S ./directory/to/solution`

## Build

1. Clone repository:
```
git clone --recursive https://github.com/martis99/build.git
```
2. Refer to [build usage](https://github.com/martis99/build#usage)
