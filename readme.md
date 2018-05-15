# AssetsToC
An advanced tool for automagically generating C code from assets.

**Attention: This tool is still in development, there will be debug logs present!**

## Help
You can find a complete documentation [here](https://siphalor.github.io/AssetsToC/site).

For short help about running the `atoc` executable see `atoc --help`.

## Example
`.atoc`:

	test.txt | test > out.hpp

`test.txt`:

	Hello World!

Will be generated as `out.hpp`:

	const unsigned char[12] test = {0x48,0x65,0x6c,0x6c,0x6f,0x20,0x57,0x6f,0x72,0x6c,0x64,0x21};
	const unsigned test_size = 12;

## Installation
For the installation you need the latest [Boost](https://boost.org) with prebuilt binaries for log, filesystem and program_options version along with CMake.

Simply `mkdir build`, `cd build` and `cmake ..`. Then you can just use `make`.

