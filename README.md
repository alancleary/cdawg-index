# cdawg-index
Uses the CDAWG as an index for pattern matching on grammar-compressed strings.

## Building

The project uses the [CMake](https://cmake.org/) meta-build system to generate build files specific to your environment.
Generate the build files as follows:
```bash
cmake -B build .
```
This will create a `build/` directory containing all the build files.

To build the code in using the files in the `build/` directory, run:
```bash
cmake --build build
```
This will generate an `cdawg-index` executable in the `build/` directory.
If you make changes to the code, you only have to run this command to recompile the code.


## Running

`cdawg-index` uses a command-line interface (CLI).
Its usage instructions are as follows:
```bash
Usage: cdawg-index <command> [<args>]
```
The `<command>` accepts `index` or `search`.
`index` creates a CDAWG index for the given grammar and `search` searches the given grammar using a pre-built CDAWG index.
Run the either command to see command-specific CLI instructions.

Currently only MR-RePair and Navarro grammars are supported.
To easily parse either grammar type, the `<filename>` argument of both commands is a single filename without extensions.
For example, Navarro grammars are encoded using two files: `<filename>.C` and `<filename>.R`.
The following command would be used to generate a CDAWG index for a Navarro grammar:
```bash
./build/cdawg-index index navarro <filename>
```
