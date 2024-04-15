.include "macros/extern.s"

.set true, 1
.set false, 0

# global const string -> arg1 = name, arg2 = value
.macro gstring name:req, value:req
    cglobal \name
    sstring \name, \value
.endm

# static (local) const string -> arg1 = name, arg2 = value
.macro sstring name:req, value:req
    .section .rdata
    \name\(): .asciz "\value"
.endm

# global int -> arg1 = name, arg2 = value
.macro gint name:req, value:req
    cglobal \name
    sint \name, \value
.endm

# static (local) int -> arg1 = name, arg2 = value
.macro sint name:req, value:req
    .section .data
    \name\(): .long (\value)
.endm

# global short -> arg1 = name, arg2 = value
.macro gshort name:req, value:req
    cglobal \name
    sshort \name, \value
.endm

# static (local) short -> arg1 = name, arg2 = value
.macro sshort name:req, value:req
    .section .data
    \name\(): .short (\value)
.endm

# global byte -> arg1 = name, arg2 = value
.macro gbyte name:req, value:req
    cglobal \name
    sbyte \name, \value
.endm

# static (local) byte -> arg1 = name, arg2 = value
.macro sbyte name:req, value:req
    .section .data
    \name\(): .byte (\value)
.endm

# global bool -> arg1 = name, arg2 = value
.macro gbool name:req, value:req
    cglobal \name
    sbyte \name, \value
.endm

# static (local) bool -> arg1 = name, arg2 = value
.macro sbool name:req, value:req
    sbyte \name, \value
.endm

# global function -> arg1 = name
.macro gfunction name:req
    cglobal \name
    sfunction \name
.endm

# static (local) function -> arg1 = name
.macro sfunction name:req
    .section .text
    .align 8, 0xCC
    \name\():
.endm

# global file (const byte array) -> arg1 = name, arg2 = filePath
.macro gfile name:req, path:req
    cglobal \name
    cglobal \name\()Length
    sfile \name, \path
.endm

# static (local) file (const byte array) -> arg1 = name, arg2 = filePath
.macro sfile name:req, path:req
    .section .rdata
_incbin_start_\@:
    \name\(): .incbin "\path"
_incbin_end_\@: 
    \name\()Length: .long (_incbin_end_\@ - _incbin_start_\@)
.endm
