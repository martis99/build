# build

build - cross-platform build system generator 

***

### Platforms
- [x] Windows
- [x] Linux

### Build systems
- [x] Visual Studio 17
- [x] CMake
- [x] Make
- [x] VS Code
- [ ] QMake
- [ ] Ninja
- [ ] Premake
- [ ] Meson

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
