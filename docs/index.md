# AssetsToC Wiki
AssetsToC (or short *atoc*) is an advanced genration system for building C code out of asset files.

## Example
`.atoc`:

	:::atoc
	test.txt | test > out.hpp

`test.txt`:

	:::atoc
	Hello World!

Will be generated as `out.hpp`:

	:::c
	const unsigned char[12] test = {0x48,0x65,0x6c,0x6c,0x6f,0x20,0x57,0x6f,0x72,0x6c,0x64,0x21};
	const unsigned test_size = 12;
