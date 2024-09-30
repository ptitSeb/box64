# Wrapper helper

This folder is semi-independent from the parent project (`box64`). This sub-project aims to (partially) automating the generation of the private headers in `src/wrapped`. This is, however, still a work-in-progress and in alpha.

As such, **this sub-project is mainly aimed at people who know how to read code and are familiar with the wrapped libraries part of `box64`**.

## Licensing

This program is under the MIT license. However, some system header files under the LGPL license (copied from a GNU libc Arch Linux installation) have been adapted into the `include-fixed` folder; these files are not copied into the output and simply serve as data. As such, I believe this falls under fair use, and does not lead to the output of this program (used in the parent `box64` project) being under the (L)GPL license.

## Compiling

You need a C compiler and GNU Make. No library is required.

Go to this folder, then run the `make` command. This will produce a binary called `bin/wrapperhelper`.

This project has been compiled and tested with `GCC 14.2.1 20240805` on an `x86_64` machine, with no warning emitted.

You may also use the `make clean` and `make distclean` commands to remove output files (`clean`) and directories (`distclean`).

## Usage

To use the wrapper helper, run the following command in the folder containing this `README.md`:
```sh
bin/wrapperhelper -I/path/to/system/include "path_to_support_file" "path_to_private.h" "path_to_private.h"
```

You may add as many `-I` options as needed. The folders `include-override/<arch>` and `include-override/common` are always prioritized, as if they appeared first in the command line.

You may also use the `-32` and `-64` switches to generate `box32` or `box64` files respectively. Alterately, you can use the `--emu arch` and `--target arch` options to select more precisely the emlated and executing platforms, though only `x86`, `x86_64` and `aarch64` are supported for now. By default, everything is as if `-64` was supplied.

The first file is a `C` file containing every declaration required. The second file is the "requests" input. The third file is the output file, which may be a different file.

The support file may contain pragma declarations of the form
```c
#pragma wrappers type_letters c TYPE
#pragma wrappers type_letters_strict c TYPE
```
where `TYPE` is a `type-name`. The second form marks the *exact* type `TYPE` as being a complex type though with a conversion as `c` (which may be multiple characters), while the first marks the type `TYPE`, regardless of type qualifers (`_Atomic`, `const`, `restrict`, `volatile`). Meaning:
- if a parameter has type `TYPE`, the character output will be `c`;
- if a parameter has a pointer to `TYPE`, or a structure containing `TYPE`, the output will be a `GOM` function.

Declarations of the form
```c
#pragma wrappers mark_simple TAG
```
will mark the structure or union with tag `TAG`, or the structure or union aliased to `TAG` by a `typedef` if no such structure exist, as simple. This means that a pointer to such a structure will have a character output of `p`.
This is not the same as making the pointer to the structure a complex type with conversion as `p` as e.g. pointers to pointers will behave differently.

System headers included (directly or indirectly) by the support file are overriden by the files in `include-fixed`.

The first three lines of the input are ignored.

A "request" is a structure containing an object name and, eventually, a default value (`GO`, `GO2` with type `vFiV` to function `xxx`, `DATA`...) and/or a "solved" value (which is similar, but deduced from the support file).

Valid requests (in the reference file) are:
```c
{GO/GOM/GOW/GOWM} ( name , type )
{GOD/GO2/GOWD/GOW2} ( name , type , name )
// {GO/GOM/GOW/GOWM} ( name ,
// {GO/GOM/GOW/GOWM} ( name , type )
// {GOD/GO2/GOWD/GOW2} ( name ,
// {GOD/GO2/GOWD/GOW2} ( name , type , name )
DATA[V/B/M] ( name , int )
// DATA[V/B/M] ( name ,
// DATA[V/B/M] ( name , int )
```
(where `{A/B}` means `A` or `B` and `[A/B]` means `A`, `B` or nothing). All other comments are ignored.

If you want to explore the output of the different stages of the helper, you can use the following forms:
```sh
bin/wrapperhelper --prepare "path_to_support_file" # (1)
bin/wrapperhelper --preproc "path_to_support_file" # (2)
bin/wrapperhelper --proc "path_to_support_file"    # (3)
bin/wrapperhelper "path_to_support_file"           # (3) as well
```
1. This form outputs the list of preprocessor tokens (the "post-prepare" phase).
2. This form outputs the list of processor tokens (the "post-preprocessor" phase).
3. This form outputs the list of constants, type definitions, structure definitions, and declarations (the "post-processor" phase).

### Example

To remake the `wrappedlibc_private.h` file, use the following command:
```sh
bin/wrapperhelper example-libc.h ../src/wrapped/wrappedlibc_private.h ../src/wrapped/wrappedlibc_private.h
```
This will emit a few marnings and (non-fatal) errors, then write the result directly in `wrappedlibc_private.h`.

## Maintaining

All of the source code is included in the `src` folder.

The `main` function is in `main.c`.

The first phase of compilation (steps 1-3 and a part of step 5 of the translation phases) is implemented in `prepare.c`.

The second phase of compilation (steps 4 and 6) is implemented in `preproc.c`.

The third phase of compilation (step 7) is implemented in `parse.c`, though no actual parsing of function definitions takes place.

The reading and writing of the `_private.h` files is implemented in `generator.c`.

## Known issues

Only native structures are read. This means that the current version of `wrapperhelper` does not detect an issue when a structure has different members or alignments in two different architectures.

No checking of signatures under `#ifdef`s is made.

Phase 5 is partially implemented, but could be greatly improved.

The following features are missing from the generator:
- Structures with at least two elements as a parameter
- Large structure as a return type (more than 16 bytes on 64bits, or 8 bytes on 32bits)
- Atomic types

The following features are missing from the preprocessor:
- General token concatenation (though the concatenation of two `PTOK_IDENT` works without issue)
- Stringify
- Skipped unexpected token warnings

The following features are missing from the parser:
- `_Alignas(type-name)` and `_Alignas(constant-expression)`
- `(type-name){initializer-list}`
- Old style function declarations
- Function definitions are ignored, not parsed
- Attributes are ignored everywhere (with a  `#define __attribute__(_)`)
