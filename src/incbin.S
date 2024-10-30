    .global _res_proxy_readme_txt
    .global res_proxy_readme_txt
    .global _res_proxy_res_rc
    .global res_proxy_res_rc
    .global _res_proxy_vcxproj
    .global res_proxy_vcxproj
    .global _res_gitignore
    .global res_gitignore
    .global _res_build_cmd
    .global res_build_cmd
    .global _res_readme_md
    .global res_readme_md
    .global _res_src_winmain_c
    .global res_src_winmain_c
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

_res_proxy_readme_txt:
res_proxy_readme_txt:
    .incbin "res/proxy/readme.txt"
    .byte 0

_res_proxy_res_rc:
res_proxy_res_rc:
    .incbin "res/proxy/res.rc"
    .byte 0

_res_proxy_vcxproj:
res_proxy_vcxproj:
    .incbin "res/proxy/proxy.vcxproj"
    .byte 0

_res_gitignore:
res_gitignore:
    .incbin "res/.gitignore"
    .byte 0

_res_build_cmd:
res_build_cmd:
    .incbin "res/build.cmd"
    .byte 0

_res_readme_md:
res_readme_md:
    .incbin "res/README.md"
    .byte 0

_res_src_winmain_c:
res_src_winmain_c:
    .incbin "res/src/winmain.cpp"
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
