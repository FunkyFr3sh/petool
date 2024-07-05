
    .global _res_gitignore
    .global res_gitignore
    .global _res_build_cmd
    .global res_build_cmd
    .global _res_src_imports_dummy_c
    .global res_src_imports_dummy_c
    .global _res_src_imports_LoadLibraryA_c
    .global res_src_imports_LoadLibraryA_c
    .global _res_src_imports_LoadLibraryExA_c
    .global res_src_imports_LoadLibraryExA_c
    .global _res_src_imports_LoadLibraryA_GetProcAddress_c
    .global res_src_imports_LoadLibraryA_GetProcAddress_c
    .global _res_src_imports_LoadLibraryW_c
    .global res_src_imports_LoadLibraryW_c
    .global _res_src_imports_LoadLibraryExW_c
    .global res_src_imports_LoadLibraryExW_c
    .global _res_src_imports_LoadLibraryW_GetProcAddress_c
    .global res_src_imports_LoadLibraryW_GetProcAddress_c
    .global _res_src_imports_GetModuleHandleA_c
    .global res_src_imports_GetModuleHandleA_c
    .global _res_src_imports_GetModuleHandleA_GetProcAddress_c
    .global res_src_imports_GetModuleHandleA_GetProcAddress_c
    .global _res_src_imports_GetModuleHandleW_c
    .global res_src_imports_GetModuleHandleW_c
    .global _res_src_imports_GetModuleHandleW_GetProcAddress_c
    .global res_src_imports_GetModuleHandleW_GetProcAddress_c
    .global _res_readme_md
    .global res_readme_md
    .global _res_src_start_c
    .global res_src_start_c
    .global _res_inc_imports_h
    .global res_inc_imports_h
    .global _res_inc_app_h
    .global res_inc_app_h
    .global _res_inc_app_inc
    .global res_inc_app_inc
    .global _res_inc_patch_h
    .global res_inc_patch_h
    .global _res_inc_macros_datatypes_inc
    .global res_inc_macros_datatypes_inc
    .global _res_inc_macros_datatypes_s
    .global res_inc_macros_datatypes_s
    .global _res_inc_macros_extern_inc
    .global res_inc_macros_extern_inc
    .global _res_inc_macros_extern_s
    .global res_inc_macros_extern_s
    .global _res_inc_macros_patch_h
    .global res_inc_macros_patch_h
    .global _res_inc_macros_patch_inc
    .global res_inc_macros_patch_inc
    .global _res_inc_macros_setsym_h
    .global res_inc_macros_setsym_h
    .global _res_inc_macros_patch_s
    .global res_inc_macros_patch_s

    .data

_res_gitignore:
res_gitignore:
    .incbin "res/.gitignore"
    .byte 0

_res_build_cmd:
res_build_cmd:
    .incbin "res/build.cmd"
    .byte 0

_res_src_imports_dummy_c:
res_src_imports_dummy_c:
    .incbin "res/src/imports_dummy.c"
    .byte 0

_res_src_imports_LoadLibraryA_c:
res_src_imports_LoadLibraryA_c:
    .incbin "res/src/imports_LoadLibraryA.c"
    .byte 0

_res_src_imports_LoadLibraryExA_c:
res_src_imports_LoadLibraryExA_c:
    .incbin "res/src/imports_LoadLibraryExA.c"
    .byte 0

_res_src_imports_LoadLibraryA_GetProcAddress_c:
res_src_imports_LoadLibraryA_GetProcAddress_c:
    .incbin "res/src/imports_LoadLibraryA_GetProcAddress.c"
    .byte 0

_res_src_imports_LoadLibraryW_c:
res_src_imports_LoadLibraryW_c:
    .incbin "res/src/imports_LoadLibraryW.c"
    .byte 0

_res_src_imports_LoadLibraryExW_c:
res_src_imports_LoadLibraryExW_c:
    .incbin "res/src/imports_LoadLibraryExW.c"
    .byte 0

_res_src_imports_LoadLibraryW_GetProcAddress_c:
res_src_imports_LoadLibraryW_GetProcAddress_c:
    .incbin "res/src/imports_LoadLibraryW_GetProcAddress.c"
    .byte 0

_res_src_imports_GetModuleHandleA_c:
res_src_imports_GetModuleHandleA_c:
    .incbin "res/src/imports_GetModuleHandleA.c"
    .byte 0

_res_src_imports_GetModuleHandleA_GetProcAddress_c:
res_src_imports_GetModuleHandleA_GetProcAddress_c:
    .incbin "res/src/imports_GetModuleHandleA_GetProcAddress.c"
    .byte 0

_res_src_imports_GetModuleHandleW_c:
res_src_imports_GetModuleHandleW_c:
    .incbin "res/src/imports_GetModuleHandleW.c"
    .byte 0

_res_src_imports_GetModuleHandleW_GetProcAddress_c:
res_src_imports_GetModuleHandleW_GetProcAddress_c:
    .incbin "res/src/imports_GetModuleHandleW_GetProcAddress.c"
    .byte 0

_res_readme_md:
res_readme_md:
    .incbin "res/README.md"
    .byte 0

_res_src_start_c:
res_src_start_c:
    .incbin "res/src/start.c"
    .byte 0

_res_inc_imports_h:
res_inc_imports_h:
    .incbin "res/inc/imports.h"
    .byte 0

_res_inc_app_h:
res_inc_app_h:
    .incbin "res/inc/app.h"
    .byte 0

_res_inc_app_inc:
res_inc_app_inc:
    .incbin "res/inc/app.inc"
    .byte 0

_res_inc_patch_h:
res_inc_patch_h:
    .incbin "res/inc/patch.h"
    .byte 0

_res_inc_macros_datatypes_inc:
res_inc_macros_datatypes_inc:
    .incbin "res/inc/macros/datatypes.inc"
    .byte 0

_res_inc_macros_datatypes_s:
res_inc_macros_datatypes_s:
    .incbin "res/inc/macros/datatypes.s"
    .byte 0
    
_res_inc_macros_extern_inc:
res_inc_macros_extern_inc:
    .incbin "res/inc/macros/extern.inc"
    .byte 0

_res_inc_macros_extern_s:
res_inc_macros_extern_s:
    .incbin "res/inc/macros/extern.s"
    .byte 0
    
_res_inc_macros_patch_h:
res_inc_macros_patch_h:
    .incbin "res/inc/macros/patch.h"
    .byte 0
    
_res_inc_macros_patch_inc:
res_inc_macros_patch_inc:
    .incbin "res/inc/macros/patch.inc"
    .byte 0
    
_res_inc_macros_setsym_h:
res_inc_macros_setsym_h:
    .incbin "res/inc/macros/setsym.h"
    .byte 0
    
_res_inc_macros_patch_s:
res_inc_macros_patch_s:
    .incbin "res/inc/macros/patch.s"
    .byte 0
