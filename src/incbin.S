.macro INCBIN name:req, path:req
    .global \name
    .global _\name
    .data
_\name\():
\name\():
    .incbin "\path"
    .byte 0
.endm

INCBIN res_proxy_readme_txt, "res/proxy/readme.txt"
INCBIN res_proxy_res_rc, "res/proxy/res.rc"
INCBIN res_proxy_vcxproj, "res/proxy/proxy.vcxproj"
INCBIN res_gitignore, "res/.gitignore"
INCBIN res_build_cmd, "res/build.cmd"
INCBIN res_readme_md, "res/README.md"
INCBIN res_src_winmain_c, "res/src/winmain.cpp"
INCBIN res_inc_app_h, "res/inc/app.h"
INCBIN res_inc_app_inc, "res/inc/app.inc"
INCBIN res_inc_patch_h, "res/inc/patch.h"
INCBIN res_inc_macros_datatypes_inc, "res/inc/macros/datatypes.inc"
INCBIN res_inc_macros_datatypes_s, "res/inc/macros/datatypes.s"
INCBIN res_inc_macros_extern_inc, "res/inc/macros/extern.inc"
INCBIN res_inc_macros_extern_s, "res/inc/macros/extern.s"
INCBIN res_inc_macros_patch_h, "res/inc/macros/patch.h"
INCBIN res_inc_macros_patch_inc, "res/inc/macros/patch.inc"
INCBIN res_inc_macros_setsym_h, "res/inc/macros/setsym.h"
INCBIN res_inc_macros_patch_s, "res/inc/macros/patch.s"
