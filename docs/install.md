## Prerequisites
This project uses the [Boost library](https://www.boost.org) and the [CMake](https://www.cmake.org) build system.

In boost you need to compile the binaries for log, filesystem and program_options.

## Installation
1. Clone or download this project locally
2. Open the console in the project root directory
3. Use `mkdir build` to create your build directory
4. Enter this directory (`cd build`)
5. Run `cmake -G "<your C++ compiler suite>" ..` (e. g. `cmake -G "MinGW Makefiles" ..`)
6. Run `make` or `mingw32-make`
7. The atoc binary should be ready in the `bin` directory

## Get started
See [Home](..) for a simple example and [Syntax](syntax) for a more advanced look at how to use AssetsToC.
