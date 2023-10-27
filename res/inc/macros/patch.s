
.macro memset start, value, end
    .section .patch,"d0"
    .long (\start)
    .long (\end) - (\start)
    .fill (\end) - (\start), 1, (\value)
.endm

.macro memsjmp src, dst
    .section .patch,"d0"
    .long (\src)
    .long 2
    .byte 0xEB
    .byte (\dst) - (\src) - 2
.endm

.macro memljmp src, dst
    .section .patch,"d0"
    .long (\src)
    .long 5
    .byte 0xE9
    .long (\dst) - (\src) - 5
.endm

.macro memcall src, dst
    .section .patch,"d0" 
    .long (\src)
    .long 5
    .byte 0xE8
    .long (\dst) - (\src) - 5
.endm

.macro memcpy addr, inst
    .section .patch,"d0" 
    .long (\addr)
    .long (_memcpy_end_\@ - _memcpy_start_\@)
_memcpy_start_\@:
    \inst
_memcpy_end_\@:
.endm
