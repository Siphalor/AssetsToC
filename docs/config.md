## Syntax
All config lines begin with a dollar sign as their first non-whitespace character. 

In those lines the actual configuration is given as key-value-pairs.

	:::atoc
	$ key: value; foo: bar

!!! tip
	You can get a list of all available config options by using the `--help-config` argument when running the excutable in your favourite console.
	
	The list contains the config keys the corresponding value types along with the default value.

### Value types
There are currently three different value types in the config:

* string: just a basic text, nothing special
* boolean: either `#!c true` or `#!c false`
* number: an unsigned integer

Additionally some configs must not be empty or must not be zero.

## Config list
### General
* `input-buffer-size` (*integer*  - **non-zero**, default: `#!c 1024`): sets the size of the buffer for reading in files in bytes; higher values will make the program a little bit more efficient but make it use more memory
* `force-folder-rebuild` (*boolean*, default: `#!c true`): sets wether statements with folders instead of files will be forced to be rebuilt
### Directory handling
One of the most important uses for the config is to modify the input and output root directory. The config system provides three keys for that purpose.

* `copy-folder-system` (*boolean*, default: `#!c false`): sets wether to use the same subdirectory structure as given in the input when [declaring statements with folders](syntax#generation)
* `input-dir` (*string*, default: empty): appends its value to the input root directory ([passed to the executable](Execution))
* `output-dir` (*string*, default: empty): appends its value to the output root directory

??? example
		:::atoc
		$ input-dir: in; output-dir: out
		input.txt > output.h;
		input > jhkj
		# Same as in/input.txt > out/output.h

!!! tip
	Because the last two options are commonly used in the `.atoc` files there are also short declarations for them:

		:::atoc
		< new-input-directory
		> new-output-directory
### Automatic name generation
Another useful purpose is to get control over the automatic variable and filename generation.

The generator takes the relative path beginning at the current root (settable with [`input-dir` and `output-dir`](#directory-handling)) along with the file extension and uses them as "words". These words are concatened under special modifiable conditions set through the config system.

The following list contains the suffixes to use with either `array-name-` (variables) or `file-`:

* `capital` (*boolean*, default: `#!c false`): sets wether the first word should be upper case
* `delimiter` (*string*, default: empty): sets the delimiter to put beween to words
* `camelcase` (*boolean*, default: `#!c true` for variables, `#!c false` for files): sets wether the words should be printed in the camelCase style
* `include-extension` (*boolean*, default: `#!c true`): sets wether the extension is used as word
* `include-folders` (*boolean*, default: `#!c true` for variables. `#!c false` for files): sets wether the path to the file should be included as words

### File content
* `array-size` (*boolean*, default: `#!c true`): sets wether to output a size variable containing the size of the array
* `array-size-suffix` (*string*, default: `_size`): sets the suffix to put after file size variable names; e. g. (if set to `_porkchopMedia`): `#!c const unsigned variableName_porkchopMedia`
* `array-size-type` (*string*, default: `const unsigned`): sets the type to use for file size variables
* `array-type` (*string*, default: `const unsigned char`): sets the type for the main variables (the `[]` is always appended)
* `use-hex` (*boolean*, default: `#!c true`): sets wether to output the symbols in hex or decimal notation
