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

### Windows

Run `build.exe` in solution directory.  
Or execute in command line if arguments are needed:
```
./build.exe -G V
```

### Linux

In solution directory execute:
```
./build
```
  
To change build system add `-G` argument with one of the options:
| Option |   Build System   | Default on |
| ------ |----------------- | ---------- |
| V      | Visual Studio 17 | Windows    |
| C      | CMake            |            |
| M      | Make             |            |
| W      | VS Code          | Linux      |

Example:
```
./build -G C
```

## Build

1. Clone repository:
```
git clone --recursive https://github.com/martis99/build.git
```
2. Refer to [build usage](https://github.com/martis99/build#usage) but instead `build` use `mbuild`
