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
 - `setsc`  - set Characteristics for a section
 - `setts`  - set TimeDateStamp in FileHeader
 - `export` - export section data as raw binary
 - `import` - dump the import table as assembly
 - `re2obj` - convert the resource section into COFF object
 - `genmak` - generate project Makefile
 - `gensym` - generate full sym.cpp with all imports
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

To begin, generate a project by giving a path to an executable file as the only
argument. Dragging and dropping an executable on `petool.exe` also does this
on Windows and the project is generated next to the original executable in a 
subdirectory.

You should be able to re-link the executable now by executing `make` without any
modifications.

Patching
--------------------------------------------------------------------------------

Generating the patch set can be done with macros. There are macros available for 
C/C++ in `inc/macros/patch.h`, several macros for `NASM` in `inc/macros/*.inc` and
macros for `GNU as` in `inc/macros/*.s`. Runtime patching is supported as well
via `inc/patch.h` (Not recommended, unless you have no other choice).

Below are some C/C++ examples for how these macros are used in practice.

Note: All labels passed to the macros must be prefixed with _ and C++ functions 
must be defined with EXTERN_C

    /* example.cpp */

    /* This does NOT work - undefined reference to `doMagic' */
    CALL(0x410000, doMagic);
    void doMagic()
    {
       something();
    }

    /* Working example - prefixed with _ and defined with EXTERN_C */
    CALL(0x410000, _doMagic);
    EXTERN_C void doMagic()
    {
       something();
    }

    /* Working example - prefixed with _ and using asm labels instead of EXTERN_C */
    extern void doMagic() asm("_doMagic");
    CALL(0x410000, _doMagic);
    void doMagic()
    {
       something();
    }

### Call
The `CALL` macro writes a CALL instruction at _from_ to _to_. It is commonly used
to replace an existing function call with a call to your own function. This is 
the most simple and cleanest way to create a patch (No assembly required).

`CALL` can be used to replace a 5byte call instruction, `CALL_NOP` can replace a 
6byte instruction. 

Note: For functions using the watcom register calling convention you will need 
to pass the additional <arg_count> arg (C/C++ only).

    CALL(<from>, <to>);
    CALL(<from>, <to>, <arg_count>);
    CALL_NOP(<from>, <to>);
    CALL_NOP(<from>, <to>, <arg_count>);

Example:

    /* Replace function call at 0x410000 with a call to doMagic */
    CALL(0x410000, _doMagic);

    EXTERN_C void doMagic(int arg1, int arg2, int arg3)
    {
       /* insert your own code here */
       something();

       /* call the orginal function that was replaced by the patch (optional) */
       original(arg1, arg2, arg3);

       /* Note: you will have to insert "original" into sym.cpp and app.h to be able to call it */
    }

    /* Same as above, but for functions using the watcom register calling convention */
    CALL(0x410000, _doMagic, 3);

    EXTERN_C void doMagic(int arg1, int arg2, int arg3)
    {
       /* insert your own code here */
       something();

       /* call the orginal function that was replaced by the patch (optional) */
       original(arg1, arg2, arg3);

       /* Note: you will have to insert "original" into sym.cpp and app.h to be able to call it */
    }

Note: the `CALL` macro is also available for `NASM` and `GNU as` under the name `@CALL`

### Detour
The `DETOUR` macro redirects all calls to an existing function to your own 
replacement function. It does also clear (INT3) the original function.

Note: For functions using the watcom register calling convention you will need 
to pass the additional <arg_count> arg.

    DETOUR(<from>, <end>, <to>);
    DETOUR(<from>, <end>, <to>, <arg_count>);

Example:

    /* 
        Clear all bytes from the start of the function (0x410000) up to the end of the function (0x410200) 
        AND do a (far) jump from 0x410000 to label doMagic 
    */`
    DETOUR(0x410000, 0x410200, _doMagic);

    EXTERN_C void doMagic(int arg1, int arg2, int arg3)
    {
       /* insert your own code here */
       something();
    }

    /* Same as above, but for functions using the watcom register calling convention */
    DETOUR(0x410000, 0x410200, _doMagic, 3);

    EXTERN_C void doMagic(int arg1, int arg2, int arg3)
    {
       /* insert your own code here */
       something();
    }

You can also keep the original function intact by clearing only the first 
instructions (just to make enough space for the 5 byte jump to fit) and afterwards 
create a trampoline in sym.cpp to restore the instructions that were removed.

    /* Clear first two instructions AND do a (far) jump from 0x410000 to label doMagic */`
    DETOUR(0x410000, 0x410007, _doMagic);

    EXTERN_C void doMagic(int arg1, int arg2, int arg3)
    {
       /* insert your own code here */
       something();

       /* call the orginal function that was replaced by the patch (optional) */
       original(arg1, arg2, arg3);
    }

    /* sym.cpp - Create trampoline to restore the two instructions removed by DETOUR */
    TRAMPOLINE(0x410007, original, "sub esp, 8; mov eax, [esp+0x1C]");

    /* Same as above, but for functions using the watcom register calling convention */
    TRAMPOLINE(0x410007, original, "sub esp, 8; mov eax, [esp+0x1C]", 3);

Note: the `DETOUR` macro is NOT available for `NASM` and `GNU as` but the same can be done via `@HOOK` instead

### Hook
The `HOOK` macro writes a JMP instruction at _addr_. You can use `HOOK` in case
there is no Call instruction nearby that could be hooked via the `CALL` macro. 
`HOOK` creates a naked function, so be sure you save and restore the values of the registers if needed.

The `HOOK` macro with 2 args can do a additional `CLEAR`, make sure you use it in case the
replaced instructions don't have a size of 5 bytes (leftover bytes could break your disassembler).

    HOOK(<addr>);
    HOOK(<addr>, <end>);

Example:

    /* Insert a JMP at 0x410000 to your own code */
    HOOK(0x410000)
    {
        /* insert original instructions that were replaced by the patch (5 byte jump) */
        __asm("mov ecx, 0xFFFFFFFF");

        /* save the values of the registers */
        __asm("pushad");

       /* insert your own code here */
       something();

       /* restore the values of the registers and jump back to the original location */
       __asm("popad; jmp 0x410000 + 5");
    }

Note: the `HOOK` macro is also available for `NASM` and `GNU as` under the name `@HOOK`

### Set instruction
Inserts the given instruction at the chosen address.

    SETINST(<addr>, <inst>);

Example:

    SETINST(0x410000, "mov eax, 1");

Note: the `SETINST` macro is also available for `NASM` and `GNU as` under the name `@SET`

### Jump
Both short and long variants are included. No overflow
checks are done so do pre-calculate which one you need.

    LJMP(<from>, <to>);
    SJMP(<from>, <to>);

Example:

    /* Do a long jump from 0x410000 to label doMagic */`
    LJMP(0x410000, _doMagic);
    
    /* Do a short jump from 0x410000 to 0x41000F */`
    SJMP(0x410000, 0x41000F);

Note: the `LJMP` macro is also available for `NASM` and `GNU as` under the name `@LJMP`

Note: the `SJMP` macro is also available for `NASM` and `GNU as` under the name `@SJMP`

### Clear
Sets all bytes between _from_ and _to_ (not inclusive) to the 8-bit argument
_byte_.

When you make a `LJMP` or anything else over the original code that would
leave some instructions broken or a dead code block, consider clearing the area
before writing the jump. It ensures when you or someone else is following the
code in a disassembler or a debugger that they will not get confused by sudden
far jumps which have broken instructions just after them.

    CLEAR(<from>, <byte>, <to>);
    CLEAR_NOP(<from>, <to>);
    CLEAR_INT(<from>, <to>);

Example: 

    /* NOP 5 bytes starting from 0x410000 */`
    CLEAR(0x410000, 0x90, 0x410005); 

    /* Does the same as the one above, NOP 5 bytes starting from 0x410000 */`
    CLEAR_NOP(0x410000, 0x410005); 

    /* Same as CLEAR_NOP, just that it clears with INT3 instead of NOP */`
    CLEAR_INT(0x410000, 0x410005); 

Note: the `CLEAR` macro is also available for `NASM` and `GNU as` under the name `@CLEAR`

### Set values
Change values at a given location.

    SETDWORD(<addr>, <value>);
    SETWORD(<addr>, <value>);
    SETBYTE(<addr>, <value>);
    SETBYTES(<addr>, <value>);

Example:

    /* Change dword value at 0x410000 to 250000 */`
    SETDWORD(0x410000, 250000);

    /* Change word value at 0x410000 to 30000 */`
    SETWORD(0x410000, 30000);

    /* Change byte value at 0x410000 to 150 */`
    SETBYTE(0x410000, 150);

    /* Change bytes at 0x410000 to 0x146878185001 */`
    SETBYTES(0x410000, "\x14\x68\x78\x1B\x50\x01");

    /* Change string at 0x410000 to HelloWorld */`
    SETBYTES(0x410000, "HelloWorld\0");

Note: These macros are NOT available for `NASM` and `GNU as` but the same can be done via `@SET` instead

### Existing symbols in original executable (sym.cpp)

    SETCGLOB(0x004D2A80, WinMain);

When you need to refer to existing symbols inside the executable, you can export
global symbols from assembly source via the `SETCGLOB` macro. Symbols can be any named 
memory address: function, data, uninitialized variable. As long as you define 
them in sym.cpp, you can use them anywhere.

Note: For functions using the watcom register calling convention you will need 
to pass the additional <arg_count> arg.

    SETCGLOB(<addr>, <name>, <arg_count>);

    SETCGLOB(0x004D500, doMagic, 4);

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
