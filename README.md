petool
================================================================================

Tool to help rebuild and patch 32-bit Windows applications.

### Tools

 - `dump`   - dump information about section of executable
 - `genlds` - generate GNU ld script for re-linking executable
 - `pe2obj` - convert PE executable into win32 object file
 - `patch`  - apply a patch set from the .patch section
 - `setdd`  - set any DataDirectory in PE header
 - `setvs`  - set VirtualSize for a section
 - `export` - export section data as raw binary
 - `import` - dump the import table as assembly
 - `re2obj` - convert the resource section into COFF object
 - `genmak` - generate project Makefile
 - `gensym` - generate sym.asm
 - `genprj` - generate full project directory (default)

Building
--------------------------------------------------------------------------------

### Build Requirements

 - GNU make
 - GNU cc

### Compiling

Type 'make' to build the tools on Linux. On Windows you might need to adjust
some environment variables to get a successful build. The developers test with
GCC on Linux and Windows (via MinGW), but the code aims to be strictly
C99-compliant so any C99-supporting platform should suffice.

`petool` uses fixed-with numeric types wherever possible so building on both 64-
and 32-bit architectures is supported. Note however that the code currently
supports working with 32-bit portable executable.

Setting up
--------------------------------------------------------------------------------

`petool` tries its best to create a re-linkable template for any given normal
Windows executable. This is not always successful so be warned it might not 
work out of the box without modifications.

To begin, generate a project by giving a path to an executable file as the only
argument. Dragging and dropping an executable on `petool.exe` also does this
on Windows and the project is generated next to the original executable in a 
subdirectory.

For technical reasons, embedded `.bss` inside `.data` is not supported but is
instead unwound to separate `.bss` after `.data`. You can optionally use `setdd`
command to expand `.data` to its original size.

You should be able to re-link the executable now by executing `make` without any
modifications.

Patching
--------------------------------------------------------------------------------

Generating the patch set can be done with macros. There are macros available for 
C/C++ in `inc/macros/patch.h`, several macros for `NASM` in `inc/macros/*.inc` and
macros for `GNU as` in `inc/macros/patch.s`.
Below are some C/C++ examples for how these macros are used in practice.

### Jump

    LJMP(<from>, <to>);
    SJMP(<from>, <to>);

Both short (near) and long (far) variants are included. Jumping to an absolute
address is supported and is converted to relative by the linker. No overflow
checks are done so do pre-calculate which one you need.

Example: `LJMP(0x410000, doMagic); /* Do a (far) jump from 0x410000 to label doMagic */`

### Call

    CALL(<from>, <to>);

The CALL macro writes a CALL instruction at _from_ to _to_. Absolute
addresses are converted to relative by the linker.

Example: `CALL(0x410000, doMagic); /* Make a call from 0x410000 to label doMagic */`

### Clear

    CLEAR(<from>, <byte>, <to>);

Sets all bytes between _from_ and _to_ (not inclusive) to the 8-bit argument
_byte_.

When you make a `LJMP` or anything else over the original code that would
leave some instructions broken or a dead code block, consider clearing the area
before writing the jump. It ensures when you or someone else is following the
code in a disassembler or a debugger that they will not get confused by sudden
far jumps which have broken instructions just after them.

Example: `CLEAR(0x410000, 0x90, 0x410005); /* NOP 5 bytes starting from 0x410000 */`

### Existing symbols in original executable (sym.asm)

    setcglob 0x004D2A80, WinMain

When you need to refer to existing symbols inside the executable, you can export
global symbols from assembly source. Symbols can be any named memory address:
function, data, uninitialized variable. As long as you define them in sym.asm, 
you can use them anywhere. Remember decorations in the header files if you're 
referring to exported global symbols from C/C++ code.

Compiling new code
--------------------------------------------------------------------------------

You can use any compilable language that can produce an COFF or ELF object or
whatever your build of `GNU binutils` supports. `GNU as`, `GNU cc`, `GNU g++`
and `NASM` are the most compatible tools and supported by `w64devkit` out of the box.

When mixing already compiled executable with new code, you need to make sure of
the calling convention of your functions and compiler can be made compatible
with everything else.

Patch section
--------------------------------------------------------------------------------

The `patch` command reads a PE image section for patch data. The format of the
patch data is as follows:

    <dword absolute address> <dword patch length> <patch data>

These binary blobs are right after each other to form a full patch set. The
patch command simply looks up the file offset in the PE image file based on
given absolute memory address and writes over the blob at that point.

After the patch is applied, you should remove the patch section with GNU strip
as it is not needed in the final product. The default project template includes
this additional step.
